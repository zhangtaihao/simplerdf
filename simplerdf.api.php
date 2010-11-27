<?php
// $Id$
/**
 * @file
 * Simple RDF documentation
 */

/**
 * @defgroup simplerdf_plugins Simple RDF plugin API
 * @{
 * Three types of plugins can be defined:
 * - @link simplerdf_nodeuri_generators Node URI generator @endlink
 * - @link simplerdf_mappers Field RDF mapper @endlink
 * - @link simplerdf_content_field_mappers CCK field RDF mapper @endlink
 *
 * Plugin architecture overview:
 * @verbatim
 *    [Node URI generator]
 *                 ||
 *                 ||
 *                 \/
 *
 * RDF Triple: ( Subject , Predicate , Object )
 *
 *                            /\        /\
 *                  Mappings  || ___    ||
 *                            ||    |   ||
 *                          Fields  |   ||
 *                            /\    |   ||
 *                            ||    |   ||
 *                            ||   \|/  ||
 *                         [Field RDF mapper]
 *                                 /\
 *                 CCK RDF fields  ||
 *                                 ||
 *                       [CCK field RDF mapper]
 * @endverbatim
 *
 * To clarify the intent of this diagram, field RDF mappers expose node fields
 * for mapping to RDF properties in the UI. Then, when building RDF triples,
 * the specific fields are passed back to the providing mapper to map to one or
 * more objects, which are used with property (i.e. predicate) mappings for the
 * specific fields to produce the triple. CCK field RDF mappers are simply an
 * extension of the CCK mapper to produce triples tailored to the specific CCK
 * field type.
 *
 * The CTools-related information necessary to declare plugins are:
 * - $owner : this module, i.e. 'simplerdf'.
 * - $type : one of 'nodeuri_generators', 'mappers', 'content_field_mappers'.
 *
 * For detailed information on how to define these plugins, refer to the links
 * above.
 * @}
 */

/**
 * @defgroup simplerdf_themeable Theme hooks
 */

/**
 * @defgroup simplerdf_nodeuri_generators Node URI generator plugins
 */

/**
 * @defgroup simplerdf_mappers Field RDF mapper plugins
 * @{
 * This is defined as a CTools plugin (refer to CTools Example Plugin on how to
 * implement a CTools plugin). The plugin structure is as follows:
 * - '
 *
 * each triple is an array with keys 's', 'p', 'o', 's_type', 'o_type',
 * 'o_datatype', and 'o_lang', each corresponding to a component of an RDF
 * triple. For help constructing statements, see the following helper
 * functions for making arrays with relevant keys:
 * - simplerdf_make_subject() : 's', 's_type'.
 * - simplerdf_make_predicate() : 'p'.
 * - simplerdf_make_object() : 'o', 'o_type', 'o_datatype', 'o_lang'.
 * @}
 */

/**
 * @defgroup simplerdf_content_field_mappers CCK field RDF mapper plugin
 * @{
 *
 * CCK mapper
 * @}
 */
