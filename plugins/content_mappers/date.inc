<?php
// $Id$
/**
 * @file
 * Date field provider.
 */

$plugin = array(
  'title' => t('Date'),
  'field types' => array('date', 'datetime', 'datestamp'),
  'callback fields' => 'simplerdf_content_mapper_date_fields',
  'callback map' => 'simplerdf_content_mapper_date_map',
  'weight' => -10,
);

/**
 * Date fields: from and to dates.
 */
function simplerdf_content_mapper_date_fields($content_field) {
  $name = $content_field['field_name'];
  $title = $content_field['widget']['label'];

  // Base field
  $fields = array(
    $name => array(
      'title' => $title,
      'todate' => $content_field['todate'] ? 1 : 0,
    ),
  );

  // Date range
  if ($content_field['todate']) {
    $fields[$name . ':from'] = array(
      'title' => $title . ' (from date)',
      'todate' => 1,
      'date part' => 'from',
    );
    $fields[$name . ':to'] = array(
      'title' => $title . ' (to date)',
      'todate' => 1,
      'date part' => 'to',
    );
  }

  return $fields;
}

/**
 * Map link fields.
 */
function simplerdf_content_mapper_date_map($node, $uri, $property, $field, $value, $delta) {
  // Only map if value exists
  if (!$value['value']) {
    return;
  }

  // Only map if date module exists
  static $date_exists;
  if (!$date_exists) {
    if (isset($date_exists)) {
      // Date API is not available
      return;
    }
    else {
      $date_exists = module_exists('date_api');
    }
    if (!$date_exists) {
      return;
    }
  }

  // Construct date object
  static $granularities = array('year', 'month', 'day', 'hour', 'minute', 'second');
  $content_field = content_fields($field['content field']);

  // Detect consecutive granularities
  $date_granularity = array();
  foreach ($granularities as $granularity) {
    if (in_array($granularity, $content_field['granularity'])) {
      $date_granularity[$granularity] = $granularity;
    }
    else {
      // Granularity sequence broken
      break;
    }
  }

  // Build date format
  $date_format = '';
  $rdf_datatype = 'xsd:date';
  if ($date_granularity['year']) {
    $date_format .= 'Y';
  }
  if ($date_granularity['month']) {
    $date_format .= '-m';
  }
  if ($date_granularity['day']) {
    $date_format .= '-d';
  }
  if ($date_granularity['hour']) {
    // Time is specified; now determine whether to display seconds
    $rdf_datatype = 'xsd:dateTime';
    $date_format .= '\TH:i';
    if ($date_granularity['second']) {
      $date_format .= ':s';
    }
    $date_format .= 'P';
  }

  list($name, $subtype) = explode(':', $field['name']);

  // Track whether each date field component is mapped
  static $mapped = array();
  if (!isset($mapped[$node->type][$name])) {
    $mappings = simplerdf_get_mappings('node', $node->type);
    $field_mapping = array();
    // Default field
    if (!empty($mappings[$name])) {
      $field_mapping['default'] = TRUE;
    }
    // From date
    if (!empty($mappings[$name . ':from'])) {
      $field_mapping['from'] = TRUE;
    }
    // To date
    if (!empty($mappings[$name . ':to'])) {
      $field_mapping['to'] = TRUE;
    }
    // Flag all
    if ($field_mapping['default'] && $field_mapping['from'] && $field_mapping['to']) {
      $field_mapping['all'] = TRUE;
    }

    $mapped[$node->type][$name] = $field_mapping;
  }
  else {
    $field_mapping = $mapped[$node->type][$name];
  }

  static $date_nodes = array();
  $node_s = simplerdf_make_subject('uri', $uri);
  $p = simplerdf_make_predicate($property);
  $tz = timezone_open($value['timezone']);

  // Map field
  $triples = array();
  switch ($subtype) {
    case NULL:
      if (!$field_mapping['from'] && !$field_mapping['to']) {
        // No other mappings present
        $date = date_convert($value['value'], $value['date_type'], DATE_OBJECT, $value['timezone_db']);
        $date->setTimezone($tz);
        $o = simplerdf_make_object('literal', $date->format($date_format), $rdf_datatype);
        $triples[] = $node_s + $p + $o;
      }
      elseif ($field_mapping['all']) {
        // Make a BNode
        $o = simplerdf_make_object('bnode');
        $date_nodes[$node->nid][$name][$delta] = $o['o'];
        $triples[] = $node_s + $p + $o;
      }
      break;

    case 'from':
      $s = NULL;
      if (!$field_mapping['default'] && !$field_mapping['to']) {
        // No other mappings present
        $s = $node_s;
      }
      elseif ($field_mapping['all'] && $date_nodes[$node->nid][$name][$delta]) {
        // Part of a BNode
        $s = simplerdf_make_subject('bnode', $date_nodes[$node->nid][$name][$delta]);
      }
      // Mappable "from" date
      if ($s) {
        $date = date_convert($value['value'], $value['date_type'], DATE_OBJECT, $value['timezone_db']);
        $date->setTimezone($tz);
        $o = simplerdf_make_object('literal', $date->format($date_format), $rdf_datatype);
        $triples[] = $s + $p + $o;
      }
      break;

    case 'to':
      if ($field_mapping['all'] && $date_nodes[$node->nid][$name][$delta] && $value['value2']) {
        // Map to BNode
        $s = simplerdf_make_subject('bnode', $date_nodes[$node->nid][$name][$delta]);
        $date = date_convert($value['value2'], $value['date_type'], DATE_OBJECT, $value['timezone_db']);
        $date->setTimezone($tz);
        $o = simplerdf_make_object('literal', $date->format($date_format), $rdf_datatype);
        $triples[] = $s + $p + $o;
      }
      break;
  }

  return $triples;
}
