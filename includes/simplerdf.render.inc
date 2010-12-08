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
    $triples = simplerdf_build_triples($node);
    $triples = simplerdf_render_process_triples($triples);
    $document = simplerdf_render_build_document($triples);
    if ($document) {
      cache_set('node:document:' . $node->nid, $document, 'cache_simplerdf');
    }
  }

  if ($document) {
    // Display document
    drupal_set_header('Content-Type: ' . $document['type']);
    drupal_set_header('Content-Disposition: inline; filename=node-' . $node->nid . '.rdf');
    echo $document['content'];
    exit;
  }
  else {
    // Document could not be built
    drupal_not_found();
  }
}

/**
 * Build RDF document from RDF triples.
 *
 * @param array $triples
 *   Array of processed RDF triples.
 *
 * @return array
 *   RDF document, or NULL if cannot be built. The document array consists of:
 *   - 'type' : MIME type.
 *   - 'content' : document content.
 */
function simplerdf_render_build_document($triples) {
  $namespaces = simplerdf_get_namespaces();

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

/**
 * Process raw RDF triples for output to ARC2.
 *
 * @param array $triples
 *   Array of raw RDF triples.
 *
 * @return array
 *   Triples suitable for ARC2.
 */
function simplerdf_render_process_triples($triples) {
  // Expand known namespace prefixes
  foreach ($triples as &$triple) {
    // Subject
    if ($triple['s_type'] == 'uri') {
      $value = _simplerdf_expand_ns_prefix($triple['s']);
      if (isset($value)) {
        $triple['s'] = $value;
      }
    }
    // Predicate
    $value = _simplerdf_expand_ns_prefix($triple['p']);
    if (isset($value)) {
      $triple['p'] = $value;
    }
    // Object
    if ($triple['o_type'] == 'uri') {
      $value = _simplerdf_expand_ns_prefix($triple['o']);
      if (isset($value)) {
        $triple['o'] = $value;
      }
    }
    if ($triple['o_datatype']) {
      $datatype = _simplerdf_expand_ns_prefix($triple['o_datatype']);
      if (isset($datatype)) {
        $triple['o_datatype'] = $datatype;
      }
    }
  }
  unset($triple);

  return $triples;
}

/**
 * Expand a prefixed value to its full URI
 */
function _simplerdf_expand_ns_prefix($value) {
  if (($prefix = _simplerdf_render_extract_ns_prefix($value)) && ($ns = simplerdf_get_namespaces($prefix))) {
    return $ns . substr($value, strlen($prefix) + 1);
  }
}

/**
 * Extract a possible namespace prefix.
 */
function _simplerdf_render_extract_ns_prefix($value) {
  static $protocols = array('http', 'https', 'ftp', 'news', 'nntp', 'telnet', 'mailto', 'irc', 'ssh', 'sftp', 'webcal', 'rtsp');

  // Match only against possible protocol/prefix
  if ($segment_length = strcspn($value, ':')) {
    $segment = substr($value, 0, $segment_length);
    // Only return if not a protocol
    if (!in_array($segment, $protocols)) {
      return $segment;
    }
  }
}
