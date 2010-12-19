<?php
// $Id$
/**
 * @file
 * Userreference field provider.
 */

$plugin = array(
  'title' => t('Userreference'),
  'field types' => array('userreference'),
  'callback map' => 'simplerdf_content_mapper_userreference_map',
  'weight' => -10,
);

/**
 * Map userreference uid value.
 */
function simplerdf_content_mapper_userreference_map($node, $nodeuri, $property, $field, $value) {
  // Map a non-empty value
  if ($value['uid']) {
    $s = simplerdf_make_subject('uri', $nodeuri);
    $p = simplerdf_make_predicate($property);
    $o = simplerdf_make_object('uri', simplerdf_get_uri('user', $value['uid'], user_load($value['uid'])));
    return array($s + $p + $o);
  }
}
