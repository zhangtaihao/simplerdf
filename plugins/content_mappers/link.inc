<?php
// $Id$
/**
 * @file
 * Link field provider.
 */

$plugin = array(
  'title' => t('Link'),
  'field types' => array('link'),
  'callback fields' => 'simplerdf_content_mapper_link_fields',
  'callback map' => 'simplerdf_content_mapper_link_map',
);

/**
 * Link fields: both the URL and title should be mappable.
 */
function simplerdf_content_mapper_link_fields($content_field) {
  $name = $content_field['field_name'];
  $title = $content_field['widget']['label'];
  return array(
    $name => array(
      'title' => $title . ' (link URL)',
    ),
    $name . ':title' => array(
      'title' => $title . ' (link title)',
    ),
  );
}

/**
 * Map link fields.
 */
function simplerdf_content_mapper_link_map($node, $nodeuri, $property, $field, $value) {
  list($name, $subtype) = explode(':', $field['name']);

  $s = simplerdf_make_subject('uri', $nodeuri);
  $p = simplerdf_make_predicate($property);
  $triples = array();

  switch ($subtype) {
    case NULL:
      // Link URL
      if (!empty($value['url'])) {
        $o = simplerdf_make_object('uri', $value['url']);
        $triples[] = $s + $p + $o;
      }
      break;

    case 'title':
      // Link title
      if (!empty($value['url']) && !empty($value['title'])) {
        $link = simplerdf_make_subject('uri', $value['url']);
        $o = simplerdf_make_object('literal', $value['title']);
        $triples[] = $link + $p + $o;
      }
      break;
  }

  return $triples;
}
