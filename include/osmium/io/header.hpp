#ifndef OSMIUM_IO_HEADER_HPP
#define OSMIUM_IO_HEADER_HPP

/*

This file is part of Osmium (http://osmcode.org/osmium).

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <string>
#include <osmium/osm/bounds.hpp>

namespace osmium {

    namespace io {

        /**
        * Meta information from the header of an OSM file.
        */
        class Header {

            /// Bounding box
            Bounds m_bounds;

            /**
            * Are there possibly multiple versions of the same object in this stream of objects?
            * This is true for history files and for change files, but not for normal OSM files.
            */
            bool m_has_multiple_object_versions = false;

            /// Program that generated this file.
            std::string m_generator;

            /**
             * Are nodes in the PBF files encoded in "Dense" format.
             * This is only valid for PBF files.
             */
            bool m_pbf_has_dense_nodes = false;

        public:

            Header() = default;

            Header(const Header&) = default;
            Header& operator=(const Header&) = default;

            Header(Header&&) = default;
            Header& operator=(Header&&) = default;

            ~Header() = default;

            Bounds& bounds() {
                return m_bounds;
            }

            const Bounds& bounds() const {
                return m_bounds;
            }

            Header& bounds(const Bounds& bounds) {
                m_bounds = bounds;
                return *this;
            }

            bool has_multiple_object_versions() const {
                return m_has_multiple_object_versions;
            }

            Header& has_multiple_object_versions(bool h) {
                m_has_multiple_object_versions = h;
                return *this;
            }

            const std::string& generator() const {
                return m_generator;
            }

            Header& generator(const std::string& generator) {
                m_generator = generator;
                return *this;
            }

            bool pbf_has_dense_nodes() const {
                return m_pbf_has_dense_nodes;
            }

            Header& pbf_has_dense_nodes(bool h) {
                m_pbf_has_dense_nodes = h;
                return *this;
            }

        }; // class Header

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_HEADER_HPP
