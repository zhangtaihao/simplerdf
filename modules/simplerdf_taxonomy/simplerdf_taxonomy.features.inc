<?php
// $Id$
/**
 * @file
 * Features integration.
 */

/**
 * Define RDF term exportable.
 */
function _simplerdf_taxonomy_features_api() {
  $path = drupal_get_path('module', 'simplerdf_taxonomy');

  // Simple RDF term extras
  $components['simplerdf_taxonomy_term'] = array(
    'name' => t('Simple RDF taxonomy terms'),
    'default_hook' => 'simplerdf_taxonomy_default_terms',
    'default_file' => FEATURES_DEFAULTS_INCLUDED,
    'features_source' => TRUE,
    'file' => $path . '/simplerdf_taxonomy.features.inc',
  );

  return $components;
}

/**
 * Implements hook_features_export() for simplerdf_taxonomy_term.
 */
function simplerdf_taxonomy_term_features_export($data, &$export, $module_name) {
  foreach ($data as $name) {
    $export['features']['simplerdf_taxonomy_term'][$name] = $name;
  }
  $export['dependencies']['simplerdf_taxonomy'] = 'simplerdf_taxonomy';
  return array();
}

/**
 * Implements hook_features_export_options() for simplerdf_taxonomy_term.
 */
function simplerdf_taxonomy_term_features_export_options() {
  $options = array();

  $query = 'SELECT t.tid, t.name, v.name AS vname, u.uuid
    FROM {simplerdf_taxonomy_term} st
    INNER JOIN {term_data} t ON st.tid = t.tid
    INNER JOIN {uuid_term_data} u ON t.tid = u.tid
    INNER JOIN {vocabulary} v ON t.vid = v.vid
    ORDER BY v.name ASC, t.name ASC';
  $result = db_query($query);
  while ($term = db_fetch_object($result)) {
    $options[$term->uuid] = $term->vname .' - '. $term->name;
  }

  return $options;
}

/**
 * Implements hook_features_export_render() for simplerdf_taxonomy_term.
 */
function simplerdf_taxonomy_term_features_export_render($module_name, $data, $export = NULL) {
  $code = array();
  foreach ($data as $name) {
    $query = "SELECT st.*
      FROM {simplerdf_taxonomy_term} st
      INNER JOIN {uuid_term_data} u ON st.tid = u.tid
      WHERE u.uuid = '%s'";
    $term = db_fetch_object(db_query($query, $name));
    if ($term) {
      // Export term by UUID
      $code[$name] = array(
        'remote_uri' => $term->remote_uri,
      );
    }
  }

  $code = '  return ' . features_var_export($code, '  ') . ';';
  return array('simplerdf_taxonomy_default_terms' => $code);
}

/**
 * Implements hook_features_revert() for simplerdf_taxonomy_term.
 */
function simplerdf_taxonomy_term_features_revert($module) {
  simplerdf_taxonomy_term_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild() for simplerdf_taxonomy_term.
 */
function simplerdf_taxonomy_term_features_rebuild($module) {
  if ($terms = features_get_default('simplerdf_taxonomy_term', $module)) {
    foreach ($terms as $name => $term) {
      $query = "SELECT t.tid
        FROM {term_data} t
        INNER JOIN {uuid_term_data} u ON t.tid = u.tid
        WHERE u.uuid = '%s'";
      $tid = db_result(db_query($query, $name));
      if ($tid) {
        $term = array('tid' => $tid) + $term;
        drupal_write_record('simplerdf_taxonomy_term', $term, 'tid');
        if (!db_affected_rows()) {
          drupal_write_record('simplerdf_taxonomy_term', $term);
        }
      }
    }
    // Flush cache
    simplerdf_clear_cache('term');
  }
}
