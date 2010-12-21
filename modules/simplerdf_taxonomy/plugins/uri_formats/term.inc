<?php
// $Id$
/**
 * @file
 * Term URI formatter.
 */

/**
 * Term URI formatter plugin definition.
 */
$plugin = array(
  'title' => t('Taxonomy term'),
  'description' => t('Use the term\'s path as the URI, e.g. "http://www.example.com/taxonomy/term/123", or use its remote URI if available.'),
  'object types' => array('term'),
  'aliasable' => TRUE,
  'callback format' => 'simplerdf_taxonomy_format_term_uri',
  'weight' => -20,
);

/**
 * Get the URI of a term depending on whether it is a local term.
 *
 * @param int $tid
 *   Term ID.
 *
 * @return string
 *   Term URI.
 */
function simplerdf_taxonomy_format_term_uri($type = 'term', $id, $object, array $config = array()) {
  $term = simplerdf_taxonomy_term_load($tid);
  if (empty($term->remote_uri)) {
    // Local URI
    $url_options = array('absolute' => TRUE);
    if (!$config['alias']) {
      $url_options['alias'] = TRUE;
    }
    return url('taxonomy/term/' . $term->tid, $url_options);
  }
  else {
    // Remote URI
    return $term->remote_uri;
  }
}
