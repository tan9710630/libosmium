#ifndef OSMIUM_IO_DETAIL_INPUT_FORMAT_HPP
#define OSMIUM_IO_DETAIL_INPUT_FORMAT_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2015 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <osmium/io/file.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/io/header.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/thread/queue.hpp>

namespace osmium {

    namespace io {

        namespace detail {

            using osmdata_queue_type = osmium::thread::Queue<std::future<osmium::memory::Buffer>>;
            using string_queue_type = osmium::thread::Queue<std::string>;

            class Parser {

            protected:

                string_queue_type& m_input_queue;
                osmdata_queue_type& m_output_queue;
                std::promise<osmium::io::Header>& m_header_promise;
                osmium::osm_entity_bits::type m_read_types;
                bool m_header_is_done;
                bool m_input_queue_done;

            private:

                /**
                 * Drain the given queue, ie pop and discard all values
                 * until an empty string (marking the end of file) is read.
                 */
                inline void drain_queue() {
                    std::string s;
                    do {
                        m_input_queue.wait_and_pop(s);
                    } while (!s.empty());
                }

                void send_exception(std::exception_ptr exception) {
                    std::promise<osmium::memory::Buffer> promise;
                    m_output_queue.push(promise.get_future());
                    promise.set_exception(exception);
                }

            protected:

                /**
                * Wrap the buffer into a future and add it to the output queue.
                */
                void send_to_output_queue(osmium::memory::Buffer&& buffer) {
                    std::promise<osmium::memory::Buffer> promise;
                    m_output_queue.push(promise.get_future());
                    promise.set_value(std::move(buffer));
                }

            public:

                Parser(string_queue_type& input_queue,
                       osmdata_queue_type& output_queue,
                       std::promise<osmium::io::Header>& header_promise,
                       osmium::osm_entity_bits::type read_types) :
                    m_input_queue(input_queue),
                    m_output_queue(output_queue),
                    m_header_promise(header_promise),
                    m_read_types(read_types),
                    m_header_is_done(false),
                    m_input_queue_done(false) {
                }

                Parser(const Parser&) = default;
                Parser& operator=(const Parser&) = default;

                Parser(Parser&&) = default;
                Parser& operator=(Parser&&) = default;

                virtual ~Parser() = default;

                virtual void run() = 0;

                void operator()() {
                    try {
                        run();
                    } catch (...) {
                        std::exception_ptr exception = std::current_exception();
                        if (!m_header_is_done) {
                            m_header_is_done = true;
                            m_header_promise.set_exception(exception);
                        }
                        send_exception(exception);
                    }

                    // end of file marker
                    send_to_output_queue(osmium::memory::Buffer{});

                    if (!m_input_queue_done) {
                        drain_queue();
                    }
                }

            }; // class Parser

            /**
             * This factory class is used to create objects that decode OSM
             * data written in a specified format.
             *
             * Do not use this class directly. Use the osmium::io::Reader
             * class instead.
             */
            class ParserFactory {

            public:

                typedef std::function<
                            std::unique_ptr<Parser>(
                                string_queue_type&,
                                osmdata_queue_type&,
                                std::promise<osmium::io::Header>& header_promise,
                                osmium::osm_entity_bits::type read_which_entities
                            )
                        > create_parser_type;

            private:

                typedef std::map<osmium::io::file_format, create_parser_type> map_type;

                map_type m_callbacks;

                ParserFactory() :
                    m_callbacks() {
                }

            public:

                static ParserFactory& instance() {
                    static ParserFactory factory;
                    return factory;
                }

                bool register_parser(osmium::io::file_format format, create_parser_type create_function) {
                    if (! m_callbacks.insert(map_type::value_type(format, create_function)).second) {
                        return false;
                    }
                    return true;
                }

                create_parser_type get_creator_function(const osmium::io::File& file) {
                    auto it = m_callbacks.find(file.format());
                    if (it == m_callbacks.end()) {
                        throw std::runtime_error(
                                std::string("Can not open file '") +
                                file.filename() +
                                "' with type '" +
                                as_string(file.format()) +
                                "'. No support for reading this format in this program.");
                    }
                    return it->second;
                }

            }; // class ParserFactory

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_INPUT_FORMAT_HPP
