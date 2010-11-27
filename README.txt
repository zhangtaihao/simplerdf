$Id$

  Simple RDF
=================================================

As posited by the module name, Simple RDF is designed for relatively
straightforward mapping of node fields to RDF properties for publishing in the
node's RDF document. There are several aspects of the module you can configure:

  - Mappings

    Under the administration menu for each content type, a new item called
    "Simple RDF mappings" leads to a list of fields in the content type
    available for mapping to RDF properties. Fields not mapped will not
    produce RDF triples.

  - Fields

    In the Simple RDF administration interface, you can view a list of fields
    provided by each mapper plugin. By default, there are three active plugins:
    Core node, CCK, and Custom. To define new fields, configure settings for
    the Custom mapper to expose a new node field for mapping.

  - Namespaces

    By default, namespaces available from the RDF module are available in
    Simple RDF. In addition, Simple RDF has an interface for adding namespaces
    similar to that of RDF. You can use this to define custom namespaces if you
    do not have or wish to have RDF installed. Namespaces defined using either
    module can be used in Simple RDF.

    Note: only namespaces defined in Simple RDF are available for export (see
    Exportables below).

  - Node URI

    By default, modules from the RDF suite attempts to infer the node URI from
    its default path, which in some cases is not desirable. Simple RDF allows
    you to choose the method of generating node URIs.


  Exportables
=================================================

The included modules integrate with Features and CTools Export API to make
available for export:

  - RDF property mappings
  - Custom RDF namespaces
  - Custom node fields


  Why another RDF module?
=================================================

The RDF module is designed for both importing and exporting RDF triples.
Display-wise, the RDF module is not quite, by itself, interoperable in terms of
clean metadata output. Even with RDF CCK, mappings are constrained by certain
arbitrary rules hardcoded into the module.

This module is built for Drupal 6 as a stopgap measure before Drupal 7, which
is tightly integrated with RDF. Designed merely for node RDF output, Simple RDF
can be used to customize RDF output and package configuration along with
content type setup for rapid redeployment on other Drupal 6 sites.

This module is specifically implemented to work with the RDF module as much as
possible and avoid conflicts.


  Extending the module
=================================================

For more information on extending the module to support custom components, see
API.txt in the module folder.
