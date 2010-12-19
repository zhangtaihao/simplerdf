<?php
// $Id$
/**
 * @file
 * Basic object relations API. This file is loaded with the main module file.
 */

/**
 * Register an object dependency relation.
 *
 * @param $source_type
 *   Source object type (e.g. node).
 * @param $source_id
 *   Source object ID.
 * @param $target_type
 *   Target object type (e.g. node).
 * @param $target_id
 *   Target object ID.
 *
 * @see simplerdf_rel_unregister()
 * @see simplerdf_rel_lookup()
 */
function simplerdf_rel_register($source_type, $source_id, $target_type, $target_id) {
  if ($source_type && $source_id && $target_type && $target_id) {
    @db_query("INSERT INTO {simplerdf_rel} (source_type, source_id, target_type, target_id) VALUES ('%s', '%s', '%s', '%s')", $source_type, $source_id, $target_type, $target_id);
  }
}

/**
 * Unregister object relation(s).
 *
 * @param $source_type
 *   Source object type (e.g. node).
 * @param $source_id
 *   Source object ID.
 * @param $target_type
 *   Target object type (e.g. node).
 * @param $target_id
 *   Target object ID.
 *
 * @see simplerdf_rel_register()
 * @see simplerdf_rel_lookup()
 */
function simplerdf_rel_unregister($source_type = NULL, $source_id = NULL, $target_type = NULL, $target_id = NULL) {
  $source_id = (int) $source_id;
  $target_id = (int) $target_id;
  $filters = array_filter(compact('source_type', 'source_id', 'target_type', 'target_id'));
  if ($filter_string = _simplerdf_rel_get_filter_string($filters)) {
    $filter_string = "WHERE $filter_string";
    @db_query("DELETE FROM {simplerdf_rel} $filter_string");
  }
}

/**
 * Look up object relation(s).
 *
 * @param $source_type
 *   Source object type (e.g. node).
 * @param $source_id
 *   Source object ID.
 * @param $target_type
 *   Target object type (e.g. node).
 * @param $target_id
 *   Target object ID.
 *
 * @return array
 *   Array of relation rows filtered by given arguments.
 *
 * @see simplerdf_rel_register()
 * @see simplerdf_rel_unregister()
 */
function simplerdf_rel_lookup($source_type = NULL, $source_id = NULL, $target_type = NULL, $target_id = NULL) {
  $source_id = (int) $source_id;
  $target_id = (int) $target_id;
  $filters = array_filter(compact('source_type', 'source_id', 'target_type', 'target_id'));
  if ($filter_string = _simplerdf_rel_get_filter_string($filters)) {
    $filter_string = "WHERE $filter_string";
  }
  $result = db_query("SELECT * FROM {simplerdf_rel} $filter_string");

  $rels = array();
  while ($rel = db_fetch_array($result)) {
    $rels[] = $rel;
  }
  return $rels;
}

/**
 * Prepare query filter conditions.
 */
function _simplerdf_rel_prepare_filters($raw_filters) {
  $filters = array();
  foreach (array('source_type', 'source_id', 'target_type', 'target_id') as $name) {
    if ($raw_filters[$name]) {
      $filters[$name] = $raw_filters[$name];
    }
  }
  return $filters;
}

/**
 * Render query filter string from condition array.
 */
function _simplerdf_rel_get_filter_string($filters) {
  $string = '';
  foreach ($filters as $name => $value) {
    // Prepare value
    $param = is_integer($value) ? '%d' : "'%s'";
    if (is_integer($value)) {
      $param = $value;
    }
    else {
      $param = "'" . db_escape_string($value) . "'";
    }
    // Construct string
    if (empty($string)) {
      $string = $name . ' = ' . $param;
    }
    else {
      $string .= ' AND ' . $name . ' = ' . $param;
    }
  }
  return $string;
}
