// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "rapidxml.hpp"
#include "myResource.h"

namespace my
{
    template<class Ch = char>
    class Xml //: public xml_node<Ch>, public memory_pool<Ch>
    {

    public:

        // callback interfaces in SAX
        virtual void on_start_element(const std::basic_string<Ch>& name)
        {
        }

        virtual void on_end_element(const std::basic_string<Ch>& name)
        {
        }

        virtual void on_attribute(const std::basic_string<Ch>& name, const std::basic_string<Ch>& value)
        {
        }

        virtual void on_data(const std::basic_string<Ch>& value)
        {
        }

        //! Constructs empty XML document
        Xml()
            //: xml_node<Ch>(node_document)
        {
        }

        //! Parses zero-terminated XML string according to given flags.
        //! Passed string will be modified by the parser, unless rapidxml::parse_non_destructive flag is used.
        //! The string must persist for the lifetime of the document.
        //! In case of error, rapidxml::parse_error exception will be thrown.
        //! <br><br>
        //! If you want to parse contents of a file, you must first load the file into the memory, and pass pointer to its beginning.
        //! Make sure that data is zero-terminated.
        //! <br><br>
        //! Document can be parsed into multiple times. 
        //! Each new call to parse removes previous nodes and attributes (if any), but does not clear memory pool.
        //! \param text XML data to parse; pointer is non-const to denote fact that this data may be modified by the parser.
        template<int Flags>
        void parse(std::istream& ifs)
        {
            //assert(text);

            //// Remove current contents
            //this->remove_all_nodes();
            //this->remove_all_attributes();

            // Parse BOM, if any
            parse_bom<Flags>(ifs);

            // Parse children
            while (1)
            {
                // Skip whitespace before node
                skip<whitespace_pred, Flags>(ifs, ignore_dest_pred());
                if (ifs.eof())
                    break;

                // Parse and append new child
                if (ifs.peek() == Ch('<'))
                {
                    ifs.ignore();     // Skip '<'
                    parse_node<Flags>(ifs);
                }
                else
                    throw rapidxml::parse_error("expected <", __FUNCTION__);
            }

        }

        ////! Clears the document by deleting all nodes and clearing the memory pool.
        ////! All nodes owned by document pool are destroyed.
        //void clear()
        //{
        //    this->remove_all_nodes();
        //    this->remove_all_attributes();
        //    memory_pool<Ch>::clear();
        //}

    private:

        ///////////////////////////////////////////////////////////////////////
        // Internal character utility functions

        // Detect whitespace character
        struct whitespace_pred
        {
            static unsigned char test(Ch ch)
            {
                return rapidxml::internal::lookup_tables<0>::lookup_whitespace[static_cast<unsigned char>(ch)];
            }
        };

        // Detect node name character
        struct node_name_pred
        {
            static unsigned char test(Ch ch)
            {
                return rapidxml::internal::lookup_tables<0>::lookup_node_name[static_cast<unsigned char>(ch)];
            }
        };

        // Detect attribute name character
        struct attribute_name_pred
        {
            static unsigned char test(Ch ch)
            {
                return rapidxml::internal::lookup_tables<0>::lookup_attribute_name[static_cast<unsigned char>(ch)];
            }
        };

        // Detect text character (PCDATA)
        struct text_pred
        {
            static unsigned char test(Ch ch)
            {
                return rapidxml::internal::lookup_tables<0>::lookup_text[static_cast<unsigned char>(ch)];
            }
        };

        // Detect text character (PCDATA) that does not require processing
        struct text_pure_no_ws_pred
        {
            static unsigned char test(Ch ch)
            {
                return rapidxml::internal::lookup_tables<0>::lookup_text_pure_no_ws[static_cast<unsigned char>(ch)];
            }
        };

        // Detect text character (PCDATA) that does not require processing
        struct text_pure_with_ws_pred
        {
            static unsigned char test(Ch ch)
            {
                return rapidxml::internal::lookup_tables<0>::lookup_text_pure_with_ws[static_cast<unsigned char>(ch)];
            }
        };

        // Detect attribute value character
        template<Ch Quote>
        struct attribute_value_pred
        {
            static unsigned char test(Ch ch)
            {
                if (Quote == Ch('\''))
                    return rapidxml::internal::lookup_tables<0>::lookup_attribute_data_1[static_cast<unsigned char>(ch)];
                if (Quote == Ch('\"'))
                    return rapidxml::internal::lookup_tables<0>::lookup_attribute_data_2[static_cast<unsigned char>(ch)];
                return 0;       // Should never be executed, to avoid warnings on Comeau
            }
        };

        // Detect attribute value character
        template<Ch Quote>
        struct attribute_value_pure_pred
        {
            static unsigned char test(Ch ch)
            {
                if (Quote == Ch('\''))
                    return rapidxml::internal::lookup_tables<0>::lookup_attribute_data_1_pure[static_cast<unsigned char>(ch)];
                if (Quote == Ch('\"'))
                    return rapidxml::internal::lookup_tables<0>::lookup_attribute_data_2_pure[static_cast<unsigned char>(ch)];
                return 0;       // Should never be executed, to avoid warnings on Comeau
            }
        };

        // Insert coded character, using UTF8 or 8-bit ASCII
        template<int Flags, class DestPred>
        static void insert_coded_character(DestPred& dest, unsigned long code)
        {
            if (Flags & rapidxml::parse_no_utf8)
            {
                // Insert 8-bit ASCII character
                // Todo: possibly verify that code is less than 256 and use replacement char otherwise?
                dest.push_back(static_cast<unsigned char>(code));
            }
            else
            {
                // Insert UTF8 sequence
                if (code < 0x80)    // 1 byte sequence
                {
                    dest.push_back(static_cast<unsigned char>(code));
                }
                else if (code < 0x800)  // 2 byte sequence
                {
                    dest.push_back(static_cast<unsigned char>(code >> 6 | 0xC0));
                    dest.push_back(static_cast<unsigned char>((code | 0x80) & 0xBF));
                }
                else if (code < 0x10000)    // 3 byte sequence
                {
                    dest.push_back(static_cast<unsigned char>(code >> 12 | 0xE0));
                    dest.push_back(static_cast<unsigned char>((code >> 6 | 0x80) & 0xBF));
                    dest.push_back(static_cast<unsigned char>((code | 0x80) & 0xBF));
                }
                else if (code < 0x110000)   // 4 byte sequence
                {
                    dest.push_back(static_cast<unsigned char>(code >> 18 | 0xF0));
                    dest.push_back(static_cast<unsigned char>((code >> 12 | 0x80) & 0xBF));
                    dest.push_back(static_cast<unsigned char>((code >> 6 | 0x80) & 0xBF));
                    dest.push_back(static_cast<unsigned char>((code | 0x80) & 0xBF));
                }
                else    // Invalid, only codes up to 0x10FFFF are allowed in Unicode
                {
                    throw rapidxml::parse_error("invalid numeric character entity", __FUNCTION__);
                }
            }
        }

        struct ignore_dest_pred
        {
            void push_back(Ch)
            {
                ;
            }
        };

        // Skip characters until predicate evaluates to true
        template<class StopPred, int Flags, class DestPred>
        static void skip(std::istream& ifs, DestPred& dest)
        {
            // ! EOF;
            while (StopPred::test(ifs.peek()))
                dest.push_back(ifs.get());
        }

        // Skip characters until predicate evaluates to true while doing the following:
        // - replacing XML character entity references with proper characters (&apos; &amp; &quot; &lt; &gt; &#...;)
        // - condensing whitespace sequences to single space character
        template<class StopPred, class StopPredPure, int Flags, class DestPred>
        static void skip_and_expand_character_refs(std::istream& ifs, DestPred& dest)
        {
            // If entity translation, whitespace condense and whitespace trimming is disabled, use plain skip
            if (Flags & rapidxml::parse_no_entity_translation &&
                !(Flags & rapidxml::parse_normalize_whitespace) &&
                !(Flags & rapidxml::parse_trim_whitespace))
            {
                skip<StopPred, Flags>(ifs, dest);
                return;
            }

            // Use simple skip until first modification is detected
            skip<StopPredPure, Flags>(ifs, dest);

            // Use translation skip
            my::IStreamBuff<char>& text = static_cast<my::IStreamBuff<char>&>(*ifs.rdbuf());
            while (StopPred::test(text[0]))
            {
                // If entity translation is enabled    
                if (!(Flags & rapidxml::parse_no_entity_translation))
                {
                    // Test if replacement is needed
                    if (text[0] == Ch('&'))
                    {
                        switch (text[1])
                        {

                            // &amp; &apos;
                        case Ch('a'):
                            if (text[2] == Ch('m') && text[3] == Ch('p') && text[4] == Ch(';'))
                            {
                                dest.push_back(Ch('&'));
                                ifs.ignore(5);
                                continue;
                            }
                            if (text[2] == Ch('p') && text[3] == Ch('o') && text[4] == Ch('s') && text[5] == Ch(';'))
                            {
                                dest.push_back(Ch('\''));
                                ifs.ignore(6);
                                continue;
                            }
                            break;

                            // &quot;
                        case Ch('q'):
                            if (text[2] == Ch('u') && text[3] == Ch('o') && text[4] == Ch('t') && text[5] == Ch(';'))
                            {
                                dest.push_back(Ch('"'));
                                ifs.ignore(6);
                                continue;
                            }
                            break;

                            // &gt;
                        case Ch('g'):
                            if (text[2] == Ch('t') && text[3] == Ch(';'))
                            {
                                dest.push_back(Ch('>'));
                                ifs.ignore(4);
                                continue;
                            }
                            break;

                            // &lt;
                        case Ch('l'):
                            if (text[2] == Ch('t') && text[3] == Ch(';'))
                            {
                                dest.push_back(Ch('<'));
                                ifs.ignore(4);
                                continue;
                            }
                            break;

                            // &#...; - assumes ASCII
                        case Ch('#'):
                            if (text[2] == Ch('x'))
                            {
                                unsigned long code = 0;
                                ifs.ignore(3);   // Skip &#x
                                while (1)
                                {
                                    unsigned char digit = rapidxml::internal::lookup_tables<0>::lookup_digits[static_cast<unsigned char>(text[0])];
                                    if (digit == 0xFF)
                                        break;
                                    code = code * 16 + digit;
                                    ifs.ignore();
                                }
                                insert_coded_character<Flags>(dest, code);    // Put character in output
                            }
                            else
                            {
                                unsigned long code = 0;
                                ifs.ignore(2);   // Skip &#
                                while (1)
                                {
                                    unsigned char digit = rapidxml::internal::lookup_tables<0>::lookup_digits[static_cast<unsigned char>(text[0])];
                                    if (digit == 0xFF)
                                        break;
                                    code = code * 10 + digit;
                                    ifs.ignore();
                                }
                                insert_coded_character<Flags>(dest, code);    // Put character in output
                            }
                            if (text[0] == Ch(';'))
                                ifs.ignore();
                            else
                                throw rapidxml::parse_error("expected ;", __FUNCTION__);
                            continue;

                            // Something else
                        default:
                            // Ignore, just copy '&' verbatim
                            break;

                        }
                    }
                }

                // If whitespace condensing is enabled
                if (Flags & rapidxml::parse_normalize_whitespace)
                {
                    // Test if condensing is needed                 
                    if (whitespace_pred::test(text[0]))
                    {
                        dest.push_back(Ch(' '));    // Put single space in dest
                        ifs.ignore();                      // Skip first whitespace char
                        // Skip remaining whitespace chars
                        while (whitespace_pred::test(text[0]))
                            ifs.ignore();
                        continue;
                    }
                }

                //// No replacement, only copy character
                dest.push_back(ifs.get());
            }

            //// Return new end
            //text = src;
            //return dest;

        }

        ///////////////////////////////////////////////////////////////////////
        // Internal parsing functions

        // Parse BOM, if any
        template<int Flags>
        void parse_bom(std::istream& ifs)
        {
            // UTF-8?
            my::IStreamBuff<char>& text = static_cast<my::IStreamBuff<char>&>(*ifs.rdbuf());
            if (static_cast<unsigned char>(text[0]) == 0xEF &&
                static_cast<unsigned char>(text[1]) == 0xBB &&
                static_cast<unsigned char>(text[2]) == 0xBF)
            {
                ifs.ignore(3);      // Skup utf-8 bom
            }
        }

        // Parse XML declaration (<?xml...)
        template<int Flags>
        void parse_xml_declaration(std::istream& ifs)
        {
            // If parsing of declaration is disabled
            my::IStreamBuff<char>& text = static_cast<my::IStreamBuff<char>&>(*ifs.rdbuf());
            if (!(Flags & rapidxml::parse_declaration_node))
            {
                // Skip until end of declaration
                while (text[0] != Ch('?') || text[1] != Ch('>'))
                {
                    if (!text[0])
                        throw rapidxml::parse_error("unexpected end of data", __FUNCTION__);
                    ifs.ignore();
                }
                ifs.ignore(2);    // Skip '?>'
                return;
            }

            //// Create declaration
            //xml_node<Ch>* declaration = this->allocate_node(node_declaration);

            // Skip whitespace before attributes or ?>
            skip<whitespace_pred, Flags>(ifs, ignore_dest_pred());

            // Parse declaration attributes
            parse_node_attributes<Flags>(ifs);

            // Skip ?>
            if (text[0] != Ch('?') || text[1] != Ch('>'))
                throw rapidxml::parse_error("expected ?>", __FUNCTION__);
            ifs.ignore(2);

            return;
        }

        //// Parse XML comment (<!--...)
        //template<int Flags>
        //xml_node<Ch>* parse_comment(Ch*& text)
        //{
        //    // If parsing of comments is disabled
        //    if (!(Flags & parse_comment_nodes))
        //    {
        //        // Skip until end of comment
        //        while (text[0] != Ch('-') || text[1] != Ch('-') || text[2] != Ch('>'))
        //        {
        //            if (!text[0])
        //                throw rapidxml::parse_error("unexpected end of data", text);
        //            ++text;
        //        }
        //        text += 3;     // Skip '-->'
        //        return 0;      // Do not produce comment node
        //    }

        //    // Remember value start
        //    Ch* value = text;

        //    // Skip until end of comment
        //    while (text[0] != Ch('-') || text[1] != Ch('-') || text[2] != Ch('>'))
        //    {
        //        if (!text[0])
        //            throw rapidxml::parse_error("unexpected end of data", text);
        //        ++text;
        //    }

        //    // Create comment node
        //    xml_node<Ch>* comment = this->allocate_node(node_comment);
        //    comment->value(value, text - value);

        //    // Place zero terminator after comment value
        //    if (!(Flags & parse_no_string_terminators))
        //        *text = Ch('\0');

        //    text += 3;     // Skip '-->'
        //    return comment;
        //}

        // Parse DOCTYPE
        template<int Flags>
        void parse_doctype(std::istream& ifs)
        {
            //// Remember value start
            //Ch* value = text;

            // Skip to >
            my::IStreamBuff<char>& text = static_cast<my::IStreamBuff<char>&>(*ifs.rdbuf());
            while (text[0] != Ch('>'))
            {
                // Determine character type
                switch (text[0])
                {

                    // If '[' encountered, scan for matching ending ']' using naive algorithm with depth
                    // This works for all W3C test files except for 2 most wicked
                case Ch('['):
                {
                    ifs.ignore();     // Skip '['
                    int depth = 1;
                    while (depth > 0)
                    {
                        switch (text[0])
                        {
                        case Ch('['): ++depth; break;
                        case Ch(']'): --depth; break;
                        case 0: throw rapidxml::parse_error("unexpected end of data", __FUNCTION__);
                        }
                        ifs.ignore();
                    }
                    break;
                }

                // Error on end of text
                case Ch('\0'):
                    throw rapidxml::parse_error("unexpected end of data", __FUNCTION__);

                    // Other character, skip it
                default:
                    ifs.ignore();

                }
            }

            //// If DOCTYPE nodes enabled
            //if (Flags & parse_doctype_node)
            //{
            //    // Create a new doctype node
            //    xml_node<Ch>* doctype = this->allocate_node(node_doctype);
            //    doctype->value(value, text - value);

            //    // Place zero terminator after value
            //    if (!(Flags & parse_no_string_terminators))
            //        *text = Ch('\0');

            //    text += 1;      // skip '>'
            //    return doctype;
            //}
            //else
            //{
            ifs.ignore(1);      // skip '>'
            return;
            //}

        }

        //// Parse PI
        //template<int Flags>
        //xml_node<Ch>* parse_pi(Ch*& text)
        //{
        //    // If creation of PI nodes is enabled
        //    if (Flags & parse_pi_nodes)
        //    {
        //        // Create pi node
        //        xml_node<Ch>* pi = this->allocate_node(node_pi);

        //        // Extract PI target name
        //        Ch* name = text;
        //        skip<node_name_pred, Flags>(text, ignore_dest_pred());
        //        if (text == name)
        //            throw rapidxml::parse_error("expected PI target", text);
        //        pi->name(name, text - name);

        //        // Skip whitespace between pi target and pi
        //        skip<whitespace_pred, Flags>(text, ignore_dest_pred());

        //        // Remember start of pi
        //        Ch* value = text;

        //        // Skip to '?>'
        //        while (text[0] != Ch('?') || text[1] != Ch('>'))
        //        {
        //            if (*text == Ch('\0'))
        //                throw rapidxml::parse_error("unexpected end of data", text);
        //            ++text;
        //        }

        //        // Set pi value (verbatim, no entity expansion or whitespace normalization)
        //        pi->value(value, text - value);

        //        // Place zero terminator after name and value
        //        if (!(Flags & parse_no_string_terminators))
        //        {
        //            pi->name()[pi->name_size()] = Ch('\0');
        //            pi->value()[pi->value_size()] = Ch('\0');
        //        }

        //        text += 2;                          // Skip '?>'
        //        return pi;
        //    }
        //    else
        //    {
        //        // Skip to '?>'
        //        while (text[0] != Ch('?') || text[1] != Ch('>'))
        //        {
        //            if (*text == Ch('\0'))
        //                throw rapidxml::parse_error("unexpected end of data", text);
        //            ++text;
        //        }
        //        text += 2;    // Skip '?>'
        //        return 0;
        //    }
        //}

        // Parse and append data
        // Return character that ends data.
        // This is necessary because this character might have been overwritten by a terminating 0
        template<int Flags>
        Ch parse_and_append_data(std::istream& ifs, std::streamoff contents_off)
        {
            // Backup to contents start if whitespace trimming is disabled
            if (!(Flags & rapidxml::parse_trim_whitespace))
                ifs.seekg(contents_off);

            // Skip until end of data
            my::IStreamBuff<char>& text = static_cast<my::IStreamBuff<char>&>(*ifs.rdbuf());
            std::basic_string<Ch> value;
            if (Flags & rapidxml::parse_normalize_whitespace)
                skip_and_expand_character_refs<text_pred, text_pure_with_ws_pred, Flags>(ifs, value);
            else
                skip_and_expand_character_refs<text_pred, text_pure_no_ws_pred, Flags>(ifs, value);

            //// Trim trailing whitespace if flag is set; leading was already trimmed by whitespace skip after >
            //if (Flags & rapidxml::parse_trim_whitespace)
            //{
            //    if (Flags & rapidxml::parse_normalize_whitespace)
            //    {
            //        // Whitespace is already condensed to single space characters by skipping function, so just trim 1 char off the end
            //        if (*(end - 1) == Ch(' '))
            //            --end;
            //    }
            //    else
            //    {
            //        // Backup until non-whitespace character is found
            //        while (whitespace_pred::test(*(end - 1)))
            //            --end;
            //    }
            //}

            // If characters are still left between end and value (this test is only necessary if normalization is enabled)
            // Create new data node
            if (!(Flags & rapidxml::parse_no_data_nodes))
            {
                //xml_node<Ch>* data = this->allocate_node(node_data);
                //data->value(value, end - value);
                //node->append_node(data);
                on_data(value);
            }

            //// Add data to parent node if no data exists yet
            //if (!(Flags & rapidxml::parse_no_element_values))
            //    if (*node->value() == Ch('\0'))
            //        node->value(value, end - value);

            //// Place zero terminator after value
            //if (!(Flags & rapidxml::parse_no_string_terminators))
            //{
            //    Ch ch = *text;
            //    *end = Ch('\0');
            //    return ch;      // Return character that ends data; this is required because zero terminator overwritten it
            //}

            // Return character that ends data
            return text[0];
        }

        //// Parse CDATA
        //template<int Flags>
        //xml_node<Ch>* parse_cdata(Ch*& text)
        //{
        //    // If CDATA is disabled
        //    if (Flags & parse_no_data_nodes)
        //    {
        //        // Skip until end of cdata
        //        while (text[0] != Ch(']') || text[1] != Ch(']') || text[2] != Ch('>'))
        //        {
        //            if (!text[0])
        //                throw rapidxml::parse_error("unexpected end of data", text);
        //            ++text;
        //        }
        //        text += 3;      // Skip ]]>
        //        return 0;       // Do not produce CDATA node
        //    }

        //    // Skip until end of cdata
        //    Ch* value = text;
        //    while (text[0] != Ch(']') || text[1] != Ch(']') || text[2] != Ch('>'))
        //    {
        //        if (!text[0])
        //            throw rapidxml::parse_error("unexpected end of data", text);
        //        ++text;
        //    }

        //    // Create new cdata node
        //    xml_node<Ch>* cdata = this->allocate_node(node_cdata);
        //    cdata->value(value, text - value);

        //    // Place zero terminator after value
        //    if (!(Flags & parse_no_string_terminators))
        //        *text = Ch('\0');

        //    text += 3;      // Skip ]]>
        //    return cdata;
        //}

        // Parse element node
        template<int Flags>
        void parse_element(std::istream& ifs)
        {
            //// Create element node
            //xml_node<Ch>* element = this->allocate_node(node_element);
            my::IStreamBuff<char>& text = static_cast<my::IStreamBuff<char>&>(*ifs.rdbuf());

            //// Extract element name
            std::basic_string<Ch> name;
            skip<node_name_pred, Flags>(ifs, name);
            if (name.empty())
                throw rapidxml::parse_error("expected element name", __FUNCTION__);
            //element->name(name, text - name);
            on_start_element(name);

            // Skip whitespace between element name and attributes or >
            skip<whitespace_pred, Flags>(ifs, ignore_dest_pred());

            // Parse attributes, if any
            parse_node_attributes<Flags>(ifs);

            // Determine ending type
            if (text[0] == Ch('>'))
            {
                ifs.ignore();
                parse_node_contents<Flags>(ifs);
            }
            else if (text[0] == Ch('/'))
            {
                ifs.ignore();
                if (text[0] != Ch('>'))
                    throw rapidxml::parse_error("expected >", __FUNCTION__);
                ifs.ignore();
            }
            else
                throw rapidxml::parse_error("expected >", __FUNCTION__);

            //// Place zero terminator after name
            //if (!(Flags & parse_no_string_terminators))
            //    element->name()[element->name_size()] = Ch('\0');
            on_end_element(name);

            //// Return parsed element
            //return element;
        }

        // Determine node type, and parse it
        template<int Flags>
        void parse_node(std::istream& ifs)
        {
            // Parse proper node type
            my::IStreamBuff<char>& text = static_cast<my::IStreamBuff<char>&>(*ifs.rdbuf());
            switch (text[0])
            {

                // <...
            default:
                // Parse and append element node
                parse_element<Flags>(ifs);
                return;

                // <?...
            case Ch('?'):
                ifs.ignore();     // Skip ?
                if ((text[0] == Ch('x') || text[0] == Ch('X')) &&
                    (text[1] == Ch('m') || text[1] == Ch('M')) &&
                    (text[2] == Ch('l') || text[2] == Ch('L')) &&
                    whitespace_pred::test(text[3]))
                {
                    // '<?xml ' - xml declaration
                    ifs.ignore(4);      // Skip 'xml '
                    parse_xml_declaration<Flags>(ifs);
                    return;
                }
                else
                {
                    // Parse PI
                    //return parse_pi<Flags>(text);
                    return;
                }

                // <!...
            case Ch('!'):

                // Parse proper subset of <! node
                switch (text[1])
                {

                    // <!-
                case Ch('-'):
                    if (text[2] == Ch('-'))
                    {
                        // '<!--' - xml comment
                        ifs.ignore(3);     // Skip '!--'
                        //return parse_comment<Flags>(text);
                        return;
                    }
                    break;

                    // <![
                case Ch('['):
                    if (text[2] == Ch('C') && text[3] == Ch('D') && text[4] == Ch('A') &&
                        text[5] == Ch('T') && text[6] == Ch('A') && text[7] == Ch('['))
                    {
                        // '<![CDATA[' - cdata
                        ifs.ignore(8);     // Skip '![CDATA['
                        //return parse_cdata<Flags>(text);
                        return;
                    }
                    break;

                    // <!D
                case Ch('D'):
                    if (text[2] == Ch('O') && text[3] == Ch('C') && text[4] == Ch('T') &&
                        text[5] == Ch('Y') && text[6] == Ch('P') && text[7] == Ch('E') &&
                        whitespace_pred::test(text[8]))
                    {
                        // '<!DOCTYPE ' - doctype
                        ifs.ignore(9);      // skip '!DOCTYPE '
                        parse_doctype<Flags>(ifs);
                        return;
                    }

                }   // switch

                // Attempt to skip other, unrecognized node types starting with <!
                ifs.ignore();     // Skip !
                while (text[0] != Ch('>'))
                {
                    if (text[0] == 0)
                        throw rapidxml::parse_error("unexpected end of data", __FUNCTION__);
                    ifs.ignore();
                }
                ifs.ignore();     // Skip '>'

            }
        }

        // Parse contents of the node - children, data etc.
        template<int Flags>
        void parse_node_contents(std::istream& ifs)
        {
            // For all children and text
            my::IStreamBuff<char>& text = static_cast<my::IStreamBuff<char>&>(*ifs.rdbuf());
            while (1)
            {
                // Skip whitespace between > and node contents
                //Ch* contents_start = text;      // Store start of node contents before whitespace is skipped
                std::streamoff contents_off = ifs.tellg();
                skip<whitespace_pred, Flags>(ifs, ignore_dest_pred());
                Ch next_char = text[0];

                // After data nodes, instead of continuing the loop, control jumps here.
                // This is because zero termination inside parse_and_append_data() function
                // would wreak havoc with the above code.
                // Also, skipping whitespace after data nodes is unnecessary.
            after_data_node:

                // Determine what comes next: node closing, child node, data node, or 0?
                switch (next_char)
                {

                    // Node closing or child node
                case Ch('<'):
                    if (text[1] == Ch('/'))
                    {
                        // Node closing
                        ifs.ignore(2);      // Skip '</'
                        if (Flags & rapidxml::parse_validate_closing_tags)
                        {
                            //// Skip and validate closing tag name
                            //Ch* closing_name = text;
                            skip<node_name_pred, Flags>(ifs, ignore_dest_pred());
                            //if (!internal::compare(node->name(), node->name_size(), closing_name, text - closing_name, true))
                            //    throw rapidxml::parse_error("invalid closing tag name", text);
                        }
                        else
                        {
                            // No validation, just skip name
                            skip<node_name_pred, Flags>(ifs, ignore_dest_pred());
                        }
                        // Skip remaining whitespace after node name
                        skip<whitespace_pred, Flags>(ifs, ignore_dest_pred());
                        if (text[0] != Ch('>'))
                            throw rapidxml::parse_error("expected >", __FUNCTION__);
                        ifs.ignore();     // Skip '>'
                        return;     // Node closed, finished parsing contents
                    }
                    else
                    {
                        // Child node
                        ifs.ignore();     // Skip '<'
                        parse_node<Flags>(ifs);
                    }
                    break;

                    // End of data - error
                case Ch('\0'):
                    throw rapidxml::parse_error("unexpected end of data", __FUNCTION__);

                    // Data node
                default:
                    next_char = parse_and_append_data<Flags>(ifs, contents_off);
                    goto after_data_node;   // Bypass regular processing after data nodes

                }
            }
        }

        // Parse XML attributes of the node
        template<int Flags>
        void parse_node_attributes(std::istream& ifs)
        {
            // For all attributes 
            my::IStreamBuff<char>& text = static_cast<my::IStreamBuff<char>&>(*ifs.rdbuf());
            while (attribute_name_pred::test(text[0]))
            {
                // Extract attribute name
                std::basic_string<Ch> name;
                name.push_back(ifs.get());     // Skip first character of attribute name
                skip<attribute_name_pred, Flags>(ifs, name);
                if (name.empty())
                    throw rapidxml::parse_error("expected attribute name", __FUNCTION__);

                //// Create new attribute
                //xml_attribute<Ch>* attribute = this->allocate_attribute();
                //attribute->name(name, text - name);
                //node->append_attribute(attribute);

                // Skip whitespace after attribute name
                skip<whitespace_pred, Flags>(ifs, ignore_dest_pred());

                // Skip =
                if (text[0] != Ch('='))
                    throw rapidxml::parse_error("expected =", __FUNCTION__);
                ifs.ignore();

                //// Add terminating zero after name
                //if (!(Flags & parse_no_string_terminators))
                //    attribute->name()[attribute->name_size()] = 0;

                // Skip whitespace after =
                skip<whitespace_pred, Flags>(ifs, ignore_dest_pred());

                // Skip quote and remember if it was ' or "
                Ch quote = text[0];
                if (quote != Ch('\'') && quote != Ch('"'))
                    throw rapidxml::parse_error("expected ' or \"", __FUNCTION__);
                ifs.ignore();

                // Extract attribute value and expand char refs in it
                std::basic_string<Ch> value;
                const int AttFlags = Flags & ~rapidxml::parse_normalize_whitespace;   // No whitespace normalization in attributes
                if (quote == Ch('\''))
                    skip_and_expand_character_refs<attribute_value_pred<Ch('\'')>, attribute_value_pure_pred<Ch('\'')>, AttFlags>(ifs, value);
                else
                    skip_and_expand_character_refs<attribute_value_pred<Ch('"')>, attribute_value_pure_pred<Ch('"')>, AttFlags>(ifs, value);

                //// Set attribute value
                //attribute->value(value, end - value);

                // Make sure that end quote is present
                if (text[0] != quote)
                    throw rapidxml::parse_error("expected ' or \"", __FUNCTION__);
                ifs.ignore();     // Skip quote

                //// Add terminating zero after value
                //if (!(Flags & parse_no_string_terminators))
                //    attribute->value()[attribute->value_size()] = 0;
                on_attribute(name, value);

                // Skip whitespace after attribute value
                skip<whitespace_pred, Flags>(ifs, ignore_dest_pred());
            }
        }

    };
}
