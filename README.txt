$Id$

  Simple RDF
=================================================

Simple RDF is a Drupal module for automatic mapping of values of Drupal objects
(e.g. node) to RDF properties. Simple RDF provides RDF mapping configuration
for the node, user, and term object types per classification, such as content
type in the case of nodes.

Simple RDF also comes with an RDF document display module, Simple RDF View.
This module publishes the RDF document for an object on a configurable path
under the object's path, e.g. node/123/rdf. You can also use the RDF module
(http://drupal.org/project/rdf) to display the RDF document, though its
capability in Drupal 6 is limited (see Limitations below).

The Simple RDF SPARQL module provides an ARC2 SPARQL endpoint for the RDF data
mapped for a site. The module maintains its own RDF triples cache optimized for
the Drupal workflow so that it always serves the latest site data while
updating itself only on site change.

Various aspects of the package are exportable via Features. Each mapping is
exportable by type (e.g. node) and classification (e.g. content type). You can
also export custom fields and namespaces.

Simple RDF is meant to be easy to use and highly extensible. For a guide to
extending the module, please see API.txt. Enjoy the power of the Semantic Web
in Drupal with Simple RDF!


  Requirements
=================================================

This module requires Drupal 6 to run. In addition, it depends on:

  - CCK
  - CTools
  - UUID

Simple RDF View and Simple RDF SPARQL require the ARC2 library. The library can
be obtained at:

  http://arc.semsol.org


  Installation
=================================================

 1. Extract the ARC2 library under sites/all/libraries such that the path
    './sites/all/libraries/arc/ARC2.php' points to the primary ARC2 file.
 2. Extract the module into sites/all/modules and enable Simple RDF!


  Usage
=================================================

There are three basic scenarios:

  - Standalone RDF publishing:
  
    You may want to use this module on its own just to publish RDF data. To 
    this end, enable Simple RDF and Simple RDF View. For taxonomy-related RDF
    mappings, also enable Simple RDF Taxonomy.
    
  - Publishing via the RDF module:
  
    If another module requires the use of RDF and provides RDF data that you
    wish to include with the RDF documents, just enable Simple RDF (and
    optionally Simple RDF Taxonomy) along with RDF and most(*) data mapped from
    Simple RDF are publishing with the other RDF triples.
    
    (*) See Limitations below.
    
  - RDF data served through SPARQL
  
    Enable the Simple RDF and Simple RDF SPARQL modules to configure mappings
    for each object type (e.g. content) and only expose the RDF data through
    the SPARQL endpoint.

Of course, the three scenarios above are only the basic use cases. Simple RDF
View is designed to work with RDF. It can take over RDF document publishing and
output on the default path; it can be configured to display RDF documents under
its own paths; or, it can display RDF documents for term (and other extra
types) and give priority over to the RDF module for node and user.

The administration interface can be found under Administer > Site building >
Simple RDF (admin/build/simplerdf).


  Limitations
=================================================

Because of how RDF is implemented (at least for Drupal 6), RDF triples output
for an object, such as a node, are limited to only triples where the subject
is the node URI itself. Additionally, the URI (node or user) must be the
default aliased path of the object.

As a workaround for the mismatched URI issue, if the URI format is not aliased
path, you can enable Simple RDF View and set the view path of, for example,
node RDF to something other than 'rdf'. The module should inject an additional
RDF document link into the HTML header and display the Simple RDF mapped data
there without interfering with the RDF module.


  API / Extending the module
=================================================

For more information on extending the module to support custom components, see
the module's wiki at:

  https://github.com/zhangtaihao/simplerdf/wiki
