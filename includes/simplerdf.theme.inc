<?php
// $Id$
/**
 * @file
 * Theme hook definitions and callbacks.
 */

/**
 * Theme hooks definitions, passthrough from hook_theme().
 */
function _simplerdf_theme() {
  $hooks['simplerdf_namespaces'] = array(
    'arguments' => array(),
    'file' => 'includes/simplerdf.theme.inc',
  );
  $hooks['simplerdf_namespaces_table'] = array(
    'arguments' => array(),
    'file' => 'includes/simplerdf.theme.inc',
  );
  $hooks['simplerdf_mappings_table'] = array(
    'arguments' => array(
      'mappings' => array(),
      'mappers' => array(),
    ),
    'file' => 'includes/simplerdf.theme.inc',
  );
  $hooks['simplerdf_admin_mappings_table'] = array(
    'file' => 'includes/simplerdf.admin.inc',
  );

  return $hooks;
}

/**
 * Display a legend for namespaces and prefixes to use in mappings.
 *
 * @return string
 *   A themed fieldset containing the namespaces table.
 *
 * @ingroup simplerdf_themeable
 */
function theme_simplerdf_namespaces() {
  $fieldset_contents = '';

  // Render namespaces table
  $fieldset_contents .= theme('simplerdf_namespaces_table');

  if (user_access('administer simplerdf') && module_exists('simplerdf_namespaces')) {
    $fieldset_contents .= '<p>' . l(t('Manage namespaces'), 'admin/build/simplerdf/namespaces') . '</p>';
  }

  $fieldset = array(
    '#title' => t('RDF namespaces'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('The table lists all namespaces defined with prefixes that can be used in mappings.'),
    '#children' => $fieldset_contents,
  );

  return theme('fieldset', $fieldset);
}

/**
 * Format available RDF namespaces and their prefixes.
 *
 * @return string
 *   An HTML table listing available namespaces.
 *
 * @ingroup simplerdf_themeable
 */
function theme_simplerdf_namespaces_table() {
  $namespaces = simplerdf_get_namespaces();

  // Build table
  $header = array(
    t('Prefix'),
    t('Namespace'),
  );
  $rows = array();

  foreach ($namespaces as $prefix => $name) {
    $rows[] = array($prefix, $name);
  }

  return theme('table', $header, $rows, array('class' => 'simplerdf-namespaces-table'));
}

/**
 * Render table for displaying RDF property mapping for each field within a
 * specific content type (i.e. the content type itself is not displayed).
 *
 * @param array $mappings
 *   RDF mappings, each an array keyed by field name with the following keys:
 *   - 'field' : Field title.
 *   - 'field_name' : Field machine name.
 *   - 'properties' : RDF properties. If an array, assumed to be a Form API element.
 *   - 'mapper' : Machine name of the mapper for mapping this field (not displayed).
 * @param array $mappers
 *   Optional. Mappers keyed by machine names. Mappings will only be grouped
 *   if these mappers are given.
 *
 * @return string
 *   Formatted table listing fields and their mappings.
 *
 * @ingroup simplerdf_themeable
 */
function theme_simplerdf_mappings_table($mappings, $mappers) {
  // Group mappings by mapper
  $groups = array();
  foreach ($mappings as $key => $mapping) {
    // Only group if mapper is defined and exists in given mappers
    $mapper = !empty($mapping['mapper']) && $mappers[$mapping['mapper']] ? $mapping['mapper'] : 0;
    $groups[$mapper][] = $key;
  }

  // Check whether there is at least one non-empty group
  $no_grouping = count($groups) == 1 && isset($groups[0]);

  // Build table
  $header = array(
    t('Field'),
    t('Machine name'),
    t('RDF properties'),
  );

  $column_count = count($header);
  $rows = array();
  foreach ($groups as $mapper => $group) {
    if (!$no_grouping) {
      // Render mapper group
      $rows[] = array(
        array(
          'data' => $mapper ? $mappers[$mapper] : t('(Other)'),
          'header' => TRUE,
          'colspan' => $column_count,
          'class' => 'simplerdf-mapper',
        ),
      );
    }
    // Render mappings
    foreach ($group as $key) {
      $mapping = $mappings[$key];
      $properties = $mapping['properties'];

      // Build properties display
      if (is_array($properties)) {
        $properties = drupal_render($properties);
      }
      else {
        $properties = filter_xss_admin($properties);
      }

      // Build row
      $rows[] = array(
        filter_xss($mapping['field_title']),
        check_plain($mapping['field_name']),
        $properties,
      );
    }
  }

  return theme('table', $header, $rows, array('class' => 'simplerdf-mappings-table'));
}
