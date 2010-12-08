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

  // Pseudo-component for exporting vocabulary mappings
  if (module_exists('strongarm')) {
    $components['simplerdf_taxonomy_mappings'] = array(
      'name' => t('Simple RDF taxonomy mappings'),
      'feature_source' => TRUE,
    );
  }

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
    simplerdf_clear_cache_all();
  }
}

/**
 * Implements hook_features_export() for simplerdf_taxonomy_mappings.
 */
function simplerdf_taxonomy_mappings_features_export($data, &$export, $module_name) {
  $pipe = array();
  if (module_exists('strongarm')) {
    foreach ($data as $name) {
      $export['features']['simplerdf_taxonomy_mappings'][$name] = $name;

      $var_name = 'simplerdf_taxonomy_mappings_' . $name;
      if (!is_null(variable_get($var_name, NULL))) {
        $pipe['variable'][$var_name] = $var_name;
      }
    }
    $export['dependencies']['simplerdf_taxonomy'] = 'simplerdf_taxonomy';
  }
  return $pipe;
}

/**
 * Implements hook_features_export_options() for simplerdf_taxonomy_mappings.
 */
function simplerdf_taxonomy_mappings_features_export_options() {
  $options = array();

  if (module_exists('strongarm')) {
    // List vocabularies with mappings
    foreach (taxonomy_get_vocabularies() as $vid => $vocabulary) {
      $name = _simplerdf_taxonomy_get_vocabulary_name($vocabulary);
      if ($name && simplerdf_taxonomy_get_mappings($vid)) {
        $options[$name] = check_plain($vocabulary->name);
      }
    }
  }

  return $options;
}
