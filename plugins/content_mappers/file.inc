<?php
// $Id$
/**
 * @file
 * File field provider.
 */

$plugin = array(
  'title' => t('File'),
  'field types' => array('filefield'),
  'callback fields' => 'simplerdf_content_mapper_file_fields',
  'callback map' => 'simplerdf_content_mapper_file_map',
  'weight' => -10,
);

/**
 * File fields: expose path, name, MIME type, size, timestamp, title, and description.
 */
function simplerdf_content_mapper_file_fields($content_field) {
  static $additions = array(
    NULL => 'URL',
    'filename' => 'name',
    'filemime' => 'MIME type',
    'filesize' => 'size',
    'timestamp' => 'timestamp',
    'title' => 'title',
    'description' => 'description',
  );

  $name = $content_field['field_name'];
  $title = $content_field['widget']['label'];
  $fields = array();
  foreach ($additions as $k => $v) {
    $field_name = $k ? ($name . ':' . $k) : $name;
    $fields[$field_name] = array(
      'title' => $title . ' (file ' . $v . ')',
    );
  }
  return $fields;
}

/**
 * Map link fields.
 */
function simplerdf_content_mapper_file_map($node, $nodeuri, $property, $field, $value) {
  $filepath = url($value['filepath'], array('absolute' => TRUE));

  list($name, $subtype) = explode(':', $field['name']);
  $triples = array();

  $p = simplerdf_make_predicate($property);
  if (!isset($subtype)) {
    // File URL field
    $s = simplerdf_make_subject('uri', $nodeuri);
    $o = simplerdf_make_object('uri', url($filepath, array('absolute' => TRUE)));
    $triples[] = $s + $p + $o;
  }
  else {
    $s = simplerdf_make_subject('uri', $filepath);

    // Detect range
    if ($subtype == 'filesize') {
      $range = 'xsd:integer';
    }
    elseif ($subtype == 'timestamp') {
      $range = 'xsd:dateTime';
    }

    // Fetch value
    switch ($subtype) {
      case 'filename':
      case 'filemime':
      case 'filesize':
        $v = $value[$subtype];
        break;

      case 'timestamp':
        $v = date('c', $value[$subtype]);
        break;

      case 'title':
      case 'description':
        $v = $value['data'][$subtype];
    }

    if (!empty($v)) {
      $o = simplerdf_make_object('literal', $v, $range);
      $triples[] = $s + $p + $o;
    }
  }

  return $triples;
}
