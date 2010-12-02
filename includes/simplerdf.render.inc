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
    $document = simplerdf_render_build_document($node);
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
function simplerdf_render_build_document($node) {
  $namespaces = simplerdf_get_namespaces();
  $triples = simplerdf_render_get_processed_triples($node);

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
 * Build RDF triples for a node for output to ARC2.
 *
 * @param stdClass $node
 *   Node object.
 *
 * @return array
 *   Triples suitable for ARC2.
 */
function simplerdf_render_get_processed_triples($node) {
  $triples = simplerdf_build_triples($node);

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
  // Calculate maximum length of protocol/prefix string
  static $max_length;
  if (!isset($max_length)) {
    $max_length = 0;
    foreach ($protocols as $v) {
      $max_length = max($max_length, strlen($v));
    }
    $namespaces = simplerdf_get_namespaces();
    unset($namespaces['_']);
    foreach (array_keys($namespaces) as $v) {
      $max_length = max($max_length, strlen($v));
    }
  }

  // Match only against possible protocol/prefix
  if ($max_length && ($segment_length = strcspn($value, ':', 0, $max_length + 1)) && $segment_length < $max_length + 1) {
    $segment = substr($value, 0, $segment_length);
    // Only return if not a protocol
    if (!in_array($segment, $protocols)) {
      return $segment;
    }
  }
}
