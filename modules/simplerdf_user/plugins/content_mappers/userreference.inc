<?php
// $Id$
/**
 * @file
 * Userreference field provider.
 */

$plugin = array(
  'title' => t('Userreference'),
  'field types' => array('userreference'),
  'callback map' => 'simplerdf_user_content_mapper_userreference_map',
  'weight' => -10,
);

/**
 * Map userreference uid value.
 */
function simplerdf_user_content_mapper_userreference_map($node, $nodeuri, $property, $field, $value) {
  // Map a non-empty value
  if ($value['uid']) {
    $s = simplerdf_make_subject('uri', $nodeuri);
    $p = simplerdf_make_predicate($property);
    $o = simplerdf_make_object('uri', simplerdf_user_get_useruri($value['uid']));
    return array($s + $p + $o);
  }
}
