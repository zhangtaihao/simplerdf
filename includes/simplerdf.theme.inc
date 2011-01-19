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
    'arguments' => array(
      'elements' => array(),
    ),
    'file' => 'includes/simplerdf.theme.inc',
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
 * Render table for displaying RDF property mapping for each field for a
 * specific object type and classification. The table contains a column for
 * field title, machine name, and configured RDF properties, optionally with
 * rows grouped by mapper. Each argument is a column in the resulting table;
 * their array sizes should match. The array keys used to primarily populate
 * the table come from $titles. Properties may use FAPI elements as values.
 *
 * @param array $titles
 *   Field titles keyed by field machine name.
 * @param array $properties
 *   Lists of properties keyed by machine name.
 * @param array $mappers
 *   Optional. Mapper names keyed by machine names. Mappings will only be
 *   grouped if this argument is given. The actual mapper titles will be
 *   automatically obtained. Note that mappers will not be displayed as a
 *   column.
 *
 * @return string
 *   Formatted table listing fields and their mappings.
 *
 * @ingroup simplerdf_themeable
 */
function theme_simplerdf_mappings_table(array $titles, array $properties, array $mappers = array()) {
  $plugins = simplerdf_get_plugins('mappers');

  // Group mappings by mapper
  $groups = array();
  $group_general = array();
  foreach ($titles as $key => $_) {
    // Only group if field key has a corresponding value in $mappers
    if (!empty($mappers[$key]) && $plugins[$mappers[$key]]) {
      $groups[$mappers[$key]][] = $key;
    }
    else {
      $group_general[] = $key;
    }
  }

  // Check whether there is at least one non-general group
  $no_grouping = empty($groups) && !empty($group_general);

  // Append general group
  if (!empty($group_general)) {
    $groups[0] = $group_general;
  }

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
          'data' => $mapper ? $plugins[$mapper]['title'] : t('(Other)'),
          'header' => TRUE,
          'colspan' => $column_count,
          'class' => 'simplerdf-mapper',
        ),
      );
    }
    // Render mappings
    foreach ($group as $key) {
      // Build row
      $rows[] = array(
        filter_xss($titles[$key]),
        check_plain($key),
        is_array($properties[$key]) ? drupal_render($properties[$key]) : filter_xss_admin($properties[$key]),
      );
    }
  }

  return theme('table', $header, $rows, array('class' => 'simplerdf-mappings-table'));
}

/**
 * Themes the Simple RDF mappings table.
 *
 * @param array $mappings
 *   Mappings form element from simplerdf_admin_mappings_form().
 *
 * @return string
 *   Fully rendered mappings table.
 *
 * @ingroup themeable
 * @ingroup simplerdf_themeable
 */
function theme_simplerdf_admin_mappings_table($elements) {
  $titles = $elements['#titles'];
  $properties = $elements['properties'];
  $mappers = $elements['#mappers'];
  if ($mappers) {
    // Group by mappers
    return theme('simplerdf_mappings_table', $titles, $properties, $mappers);
  }
  else {
    return theme('simplerdf_mappings_table', $titles, $properties);
  }
}
