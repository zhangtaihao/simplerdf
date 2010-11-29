<?php
// $Id$
/**
 * @file
 * Menu definition.
 */

/**
 * Define menu router items.
 */
function _simplerdf_menu() {
  $view_path = variable_get('simplerdf_view_path', 'rdf');
  $administer_simplerdf = array('administer simplerdf');

  // Check to make sure we aren't overriding RDF (should only happen when
  // RDF is installed after this module and path remains 'RDF').
  if ($view_path && !($view_path == 'rdf' && module_exists('rdf'))) {
    $items['node/%node/' . $view_path] = array(
      'title' => check_plain(variable_get('simplerdf_view_title', 'RDF')),
      'description' => 'View the RDF document.',
      'page callback' => 'simplerdf_render_node',
      'type' => variable_get('simplerdf_view_tab', TRUE) ? MENU_LOCAL_TASK : MENU_CALLBACK,
      'access callback' => 'node_access',
      'access arguments' => array('view', 1),
      'file' => 'includes/simplerdf.render.inc',
    );
  }

  // Node field mappings
  foreach (content_types() as $type) {
    $items['admin/content/node-type/' . $type['url_str'] . '/simplerdf'] = array(
      'title' => 'Simple RDF mappings',
      'description' => 'Configure node property mappings.',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('simplerdf_admin_mappings_form', 3),
      'type' => MENU_LOCAL_TASK,
      'access arguments' => array('administer simplerdf mappings'),
      'file' => 'includes/simplerdf.admin.inc',
      'weight' => 1,
    );
  }

  // Admin dashboard
  $items['admin/build/simplerdf'] = array(
    'title' => 'Simple RDF',
    'description' => 'Overview for Simple RDF setup.',
    'type' => MENU_NORMAL_ITEM,
    'page callback' => 'simplerdf_admin_dashboard',
    'access arguments' => $administer_simplerdf,
    'file' => 'includes/simplerdf.admin.inc',
  );
  $items['admin/build/simplerdf/dashboard'] = array(
    'title' => 'Dashboard',
    'type' => MENU_DEFAULT_LOCAL_TASK,
    'weight' => -10,
  );

  // Node URI administration
  $items['admin/build/simplerdf/nodeuri'] = array(
    'title' => 'Node URI generators',
    'description' => 'Select and configure settings for generation of node URIs for RDF documents.',
    'type' => MENU_NORMAL_ITEM,
    'page callback' => 'drupal_get_form',
    'page arguments' => array('simplerdf_admin_nodeuri_form'),
    'access arguments' => $administer_simplerdf,
    'file' => 'includes/simplerdf.admin.inc',
    'weight' => 1,
  );
  $items['admin/build/simplerdf/nodeuri/%simplerdf_nodeuri_settings'] = array(
    'title' => 'Node URI generator',
    'description' => 'Configure settings for the node URI generator.',
    'type' => MENU_CALLBACK,
    'page callback' => 'drupal_get_form',
    'page arguments' => array('simplerdf_admin_nodeuri_settings_form', 4),
    'access arguments' => $administer_simplerdf,
    'file' => 'includes/simplerdf.admin.inc',
  );

  // Settings
  $items['admin/build/simplerdf/settings'] = array(
    'title' => 'Settings',
    'description' => 'Configure general settings for Simple RDF mapping.',
    'type' => MENU_NORMAL_ITEM,
    'page callback' => 'drupal_get_form',
    'page arguments' => array('simplerdf_admin_settings_form'),
    'access arguments' => array('administer simplerdf settings'),
    'file' => 'includes/simplerdf.admin.inc',
    'weight' => 5,
  );

  // Autocomplete callbacks
  $items['simplerdf/autocomplete/%'] = array(
    'title' => 'Autocomplete',
    'description' => 'Autocomplete-enabled textfield helper.',
    'type' => MENU_CALLBACK,
    'page callback' => 'simplerdf_autocomplete',
    'page arguments' => array(2),
    'access arguments' => array('administer simplerdf'),
  );

  return $items;
}
