<?php
// $Id$
/**
 * @file
 * RDF rendering functions.
 */

/**
 * Renders a node RDF document.
 *
 * @param stdClass $node
 *   Node object.
 */
function simplerdf_render_node($node) {
  // Fetch cache
  if ($cache = cache_get('node:document:' . $node->nid, 'cache_simplerdf')) {
    $document = $cache->data;
  }
  else {
    // Build page
    $document = simplerdf_build_document($node);
    if ($document) {
      cache_set('node:document:' . $node->nid, $document, 'cache_simplerdf');
    }
  }

  if ($document) {
    // Display document
    drupal_set_header('Content-Type: ' . $document['type']);
    echo $document['content'];
    exit;
  }
  else {
    // Document could not be built
    drupal_not_found();
  }
}

/**
 * Build RDF document.
 *
 * @param stdClass $node
 *   Node object.
 *
 * @return array
 *   RDF document, or NULL if cannot be built. The document array consists of:
 *   - 'type' : MIME type.
 *   - 'content' : document content.
 */
function simplerdf_build_document($node) {
  $type = $node->type;

  // Generate node URI
  $uri = simplerdf_get_nodeuri($node);

  $fields = simplerdf_get_fields($type);
  $mappings = simplerdf_get_mappings($type);

  // Map each field
  $triples = array();
  foreach ($fields as $field_name => $field) {
    if (isset($mappings[$field_name])) {
      $triples = array_merge($triples, simplerdf_map_field($node, $uri, $mappings[$field_name], $field));
    }
  }

  // Prepare namespaces
  $namespaces = simplerdf_get_namespaces();
  unset($namespaces['_']);

  // Serialize document
  $document = array();
  module_load_include('inc', 'simplerdf', 'includes/simplerdf.arc');
  $format = variable_get('simplerdf_format', SIMPLERDF_DEFAULT_FORMAT);
  if (simplerdf_arc_include() && $serializer = simplerdf_arc_get_serializer($format, array('ns' => $namespaces))) {
    // Initialize ARC2
    $document['type'] = simplerdf_arc_get_mime_type($format, $serializer);
    $index = $serializer->toIndex($triples);

    // Lookup serialization method
    $methods = array(
      'to' . $format,
      'getSerializedIndex',
      'getSerializedArray',  // Legacy
    );

    // Serialize!
    foreach ($methods as $method) {
      if (method_exists($serializer, $method)) {
        $document['content'] = $serializer->$method($index);
        break;
      }
    }

    if (!empty($document['content'])) {
      return $document;
    }
  }
}
