<?php
// $Id$
/**
 * @file
 * Userreference field provider.
 */

$plugin = array(
  'title' => t('Userreference'),
  'field types' => array('userreference'),
  'map callback' => 'simplerdf_content_mapper_userreference_map',
  'weight' => -10,
);

/**
 * Map userreference uid value.
 */
function simplerdf_content_mapper_userreference_map($node, $uri, $property, $field, $value) {
  // Map a non-empty value
  if ($value['uid']) {
    $s = simplerdf_make_subject('uri', $uri);
    $p = simplerdf_make_predicate($property);
    $o = simplerdf_make_object('uri', simplerdf_get_uri('user', $value['uid']));
    return array($s + $p + $o);
  }
}
