<?php
// $Id$
/**
 * @file
 * Taxonomy term mapper.
 */

/**
 * Term mapper plugin definition.
 */
$plugin = array(
  'title' => t('Term'),
  'object types' => array('term'),
  'description' => t('Basic term fields as available in the Taxonomy module.'),
  'weight' => -20,
  'callback fields' => 'simplerdf_mapper_term_fields',
  'callback map' => 'simplerdf_mapper_term_map',
);

/**
 * Provide term mapping fields.
 *
 * @param string $object_type
 *   Type of object (always 'term'). This argument is ignored.
 * @param int $vid
 *   Vocabulary ID. This argument is ignored.
 *
 * @return array
 *   Mapping fields keyed by field name.
 */
function simplerdf_mapper_term_fields($object_type = 'term', $vid = NULL) {
  return array(
    'rdftype' => array('title' => t('RDF class (rdf:type)')),
    'name' => array('title' => t('Term name')),
    'description' => array('title' => t('Description')),
    'parent' => array('title' => t('Parent term')),
    'related' => array('title' => t('Related term')),
    'synonym' => array('title' => t('Term synonym')),
  );
}

/**
 * Mapping callback.
 */
function simplerdf_mapper_term_map($object_type = 'term', $term, $uri, $property, $field) {
  // Safety check: is Taxonomy actually enabled?
  static $taxonomy_exists;
  if (!isset($taxonomy_exists)) {
    $taxonomy_exists = module_exists('taxonomy');
  }
  if (!$taxonomy_exists) {
    return;
  }

  $field_name = $field['name'];
  $s = simplerdf_make_subject('uri', $uri);
  $p = simplerdf_make_predicate($property);

  $triples = array();
  switch ($field_name) {
    // RDF type
    case 'rdftype':
      $o = simplerdf_make_object('uri', $property);
      $triples[] = $s + simplerdf_make_predicate('rdf:type') + $o;
      break;

    // Single literal values
    case 'name':
    case 'description':
      if (!empty($term->$field_name)) {
        $o = simplerdf_make_object('literal', $term->$field_name);
        $triples[] = $s + $p + $o;
      }
      break;

    // Linked terms
    case 'parent':
      if (!isset($rel)) {
        // Load parent terms
        $rel = taxonomy_get_parents($term->tid);
      }
    case 'related':
      if (!isset($rel)) {
        // Load related terms
        $rel = taxonomy_get_related($term->tid);
      }

      // Map terms
      foreach ($rel as $rel_tid => $rel_term) {
        $o = simplerdf_make_object('uri', simplerdf_get_uri('term', $rel_tid));
        $triples[] = $s + $p + $o;
      }
      break;

    // Synonyms
    case 'synonym':
      $synonyms = taxonomy_get_synonyms($term->tid);
      foreach ($synonyms as $synonym) {
        $o = simplerdf_make_object('literal', $synonym);
        $triples[] = $s + $p + $o;
      }
      break;
  }

  return $triples;
}
