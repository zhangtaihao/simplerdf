<?php
// $Id$
/**
 * @file
 * Admin UI forms and callbacks.
 */

/**
 * Insert Simple RDF mappings column.
 */
function _simplerdf_content_types_overview_alter(&$header, &$rows) {
  // Operations header
  $header_index = count($header) - 1;
  $header_position = 0;
  // Trace to header column
  for ($i = 0; $i < $header_index; $i ++) {
    $header_position += $header[$i]['colspan'] ? $header[$i]['colspan'] : 1;
  }
  // Trace to column just before last in the 'Operations' group
  $header_position += $header[$header_index]['colspan'] - 1;

  // Trace the cell column index
  $cell_index = 0;
  $cell_position = 0;
  $row = $rows[0];
  foreach ($row as $cell) {
    $cell_position += $cell['colspan'] ? $cell['colspan'] : 1;
    $cell_index ++;
    if ($cell_position >= $header_position) {
      // We've reached or gone beyond the corresponding header position
      break;
    }
  }

  // Expand header
  $header[$header_index]['colspan'] = $header[$header_index]['colspan'] ? ($header[$header_index]['colspan'] + 1) : 2;

  // Insert cells
  foreach ($rows as &$row) {
    $path = 'admin/content/node-type/' . str_replace('_', '-', $row[1]) . '/simplerdf';
    array_splice($row, $cell_index, 0, l(t('RDF mappings'), $path));
  }
  unset($row);
}

/**
 * @todo ???
 * Form for configuring node property mappings.
 */
function simplerdf_admin_mappings_form($form_state, $content_type) {
  $path = drupal_get_path('module', 'simplerdf');
  drupal_add_css($path . '/simplerdf.admin.css');

  $content_type = str_replace('-', '_', $content_type);
  $form = array();

  // Cache content type
  if ($content_type) {
    $form['__content_type'] = array(
      '#type' => 'value',
      '#value' => $content_type,
    );
  }

  $form['help'] = array(
    '#type' => 'markup',
    '#prefix' => '<p>',
    '#value' => t('Enter one or more RDF property mapping (separated by commas) for each node field listed below. The fields are grouped together by their property mappers for clarity. Namespace prefixes defined for this site are listed in the table below (open the "RDF namespaces" fieldset to view). You may use these prefixes in the property mappings. Note: specified mappings will not be checked against their vocabularies.'),
    '#suffix' => '</p>',
  );
  $form['legend'] = array(
    '#type' => 'markup',
    '#value' => theme('simplerdf_namespaces'),
  );

  // Load mappings
  $rdf_mappings = simplerdf_get_mappings($content_type);

  // Build mappings
  $form['mappings'] = array(
    '#theme' => 'simplerdf_admin_mappings_table',
  );
  $fields = simplerdf_get_fields($content_type);
  foreach ($fields as $name => $field) {
    $mapping = array(
      '#field_title' => $field['title'],
      '#field_name' => $name,
      'properties' => array('#tree' => TRUE),
      '#mapper' => $field['mapper'],
    );
    // Mapping textfield
    $mapping['properties'][$name] = array(
      '#type' => 'textfield',
      '#size' => 20,
      '#maxlength' => 100,
      '#default_value' => implode(', ', $rdf_mappings[$name]),
    );

    $form['mappings'][] = $mapping;
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save mappings'),
  );

  return $form;
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
  ctools_include('plugins');

  // Prepare mappers
  $plugins = ctools_get_plugins('simplerdf', 'mappers');
  $mappers = array();
  foreach ($plugins as $name => $plugin) {
    $mappers[$name] = check_plain($plugin['title']);
  }

  foreach (element_children($elements) as $key) {
    $mapping = &$elements[$key];

    // Prepare mapping row
    $mapping['properties'] = $mapping['properties'][$mapping['#field_name']];
    $mapping['field_title'] = $mapping['#field_title'];
    $mapping['field_name'] = $mapping['#field_name'];
    $mapping['mapper'] = $mapping['#mapper'];

    // Strip properties
    foreach (element_properties($mapping) as $property) {
      unset($mapping[$property]);
    }
  }

  // Strip all form properties
  foreach (element_properties($elements) as $property) {
    unset($elements[$property]);
  }

  return theme('simplerdf_mappings_table', $elements, $mappers);
}

/**
 * Mapping form submit callback.
 */
function simplerdf_admin_mappings_form_submit($form, $form_state) {
  $values = $form_state['values'];
  $content_type = $values['__content_type'];

  // Clear mappings
  db_query("DELETE FROM {simplerdf_mappings} WHERE content_type = '%s'", $content_type);

  // Add mappings
  foreach ($values['properties'] as $field => $properties) {
    // Normalize RDF properties
    $properties = array_filter(array_map('trim', preg_split('/\s*,\s*/', $properties)));

    // Save
    if (!empty($properties)) {
      $mapping = (object) compact('content_type', 'field', 'properties');
      drupal_write_record('simplerdf_mappings', $mapping);
    }
  }

  // Clear cache
  simplerdf_clear_cache('mappings', $content_type);
  simplerdf_clear_cache('node');

  drupal_set_message(t('RDF mappings have been updated.'));
}

/**
 * Administration dashboard page.
 */
function simplerdf_admin_dashboard() {
  $path = drupal_get_path('module', 'simplerdf');
  drupal_add_css($path . '/simplerdf.admin.css');

  $output = '<div class="simplerdf-admin-dashboard">';

  $output .= '<p>' . t('An overview of the Simple RDF configuration for this site is displayed on this page.');

  $output .= '<p>' . l(t('Flush Simple RDF cache'), 'admin/build/simplerdf/flush') . '</p>';

  // Display general settings
  $output .= '<h2>' . t('General settings') . '</h2>';

  $header = array(
    t('Name'),
    t('Value'),
  );
  $rows = array();

  // Active node URI generator
  $plugins = ctools_get_plugins('simplerdf', 'nodeuri_generators');
  $active_nodeuri = variable_get('simplerdf_nodeuri_generator', SIMPLERDF_DEFAULT_NODEURI_GENERATOR);
  $rows[] = array(
    t('Node URI format'),
    check_plain($plugins[$active_nodeuri]['title']),
  );

  // View path
  $rows[] = array(
    t('RDF view path'),
    check_plain('node/%/' . variable_get('simplerdf_view_path', 'rdf')),
  );

  // Tab settings
  $rows[] = array(
    t('Show tab'),
    variable_get('simplerdf_view_tab', TRUE) ? t('true') : t('false'),
  );
  $rows[] = array(
    t('Tab title'),
    check_plain(variable_get('simplerdf_view_title', 'RDF')),
  );

  // RDF format
  module_load_include('inc', 'simplerdf', 'includes/simplerdf.arc');
  if (simplerdf_arc_include()) {
    $rows[] = array(
      t('RDF format'),
      _simplerdf_admin_get_friendly_format(variable_get('simplerdf_format', SIMPLERDF_DEFAULT_FORMAT)),
    );
  }
  else {
    drupal_set_message(t('The ARC2 library is not found. Please install it under @path to enable RDF output.', array('@path' => '<code>sites/all/libraries/arc</code>')), 'warning');
  }

  $output .= theme('table', $header, $rows, array('class' => 'simplerdf-admin-settings-table'));

  $output .= '<div class="simplerdf-admin-links">';
  $output .= l(t('Configure node URI format'), 'admin/build/simplerdf/nodeuri');
  $output .= ' | ';
  $output .= l(t('Configure settings'), 'admin/build/simplerdf/settings');
  $output .= '</div>';

  // Display namespaces
  $output .= '<h2>' . t('RDF namespaces') . '</h2>';

  $output .= theme('simplerdf_namespaces_table');
  if (module_exists('simplerdf_namespaces')) {
    $output .= '<div class="simplerdf-admin-links">';
    $output .= l(t('Manage custom namespaces'), 'admin/build/simplerdf/namespaces');
    $output .= '</div>';
  }

  // Build mappings
  $output .= '<h2>' . t('Mapped fields') . '</h2>';

  $header = array(
    t('Type'),
    t('Mapped fields'),
    t('Operations'),
  );
  $rows = array();
  foreach (node_get_types('names') as $content_type => $type_name) {
    $fields = simplerdf_get_fields($content_type);
    $rdf_mappings = simplerdf_get_mappings($content_type);
    $mapped_fields = array();
    // Group mapped fields
    foreach ($fields as $name => $field) {
      if (isset($rdf_mappings[$name])) {
        $mapped_fields[] = $name;
      }
    }
    $rows[] = array(
      check_plain($type_name),
      check_plain(implode(', ', $mapped_fields)),
      l(t('Manage mappings'), 'admin/content/node-type/' . str_replace('_', '-', $content_type) . '/simplerdf'),
    );
  }

  $output .= theme('table', $header, $rows, array('class' => 'simplerdf-admin-mapped-fields'));

  // Active plugins
  $output .= '<h2>' . t('Active plugins') . '</h2>';

  $header = array(
    t('Type'),
    t('Plugins'),
  );
  $type_names = array(
    'nodeuri_generators' => t('Node URI generator'),
    'mappers' => t('Field mappers'),
    'content_mappers' => t('Content field mappers'),
  );
  $rows = array();
  foreach ($type_names as $type => $type_name) {
    $plugin_titles = array();
    $plugins = ctools_get_plugins('simplerdf', $type);
    uasort($plugins, '_simplerdf_compare_plugin_weights');
    foreach ($plugins as $name => $plugin) {
      $plugin_titles[] = '<strong>' . check_plain($plugin['title']) . '</strong>' . ' (' . check_plain($name) . ')';
    }

    $rows[] = array(
      $type_name,
      implode(', ', $plugin_titles),
    );
  }

  $output .= theme('table', $header, $rows, array('id' => 'simplerdf-admin-active-plugins', 'class' => 'simplerdf-admin-listing-table'));

  $output .= '</div>';
  return $output;
}

/**
 * Flush Simple RDF cache and return to dashboard.
 */
function simplerdf_admin_flush() {
  simplerdf_clear_cache_all();
  drupal_set_message(t('Simple RDF caches have been cleared.'));
  drupal_goto('admin/build/simplerdf');
}

/**
 * Node URI generator choice form.
 */
function simplerdf_admin_nodeuri_form($form_state) {
  $path = drupal_get_path('module', 'simplerdf');
  drupal_add_css($path . '/simplerdf.admin.css');
  drupal_add_js($path . '/simplerdf.admin.js');

  $plugins = ctools_get_plugins('simplerdf', 'nodeuri_generators');
  uasort($plugins, '_simplerdf_compare_plugin_weights');
  $active_nodeuri = variable_get('simplerdf_nodeuri_generator', SIMPLERDF_DEFAULT_NODEURI_GENERATOR);

  // Cache old setting
  $form['__nodeuri'] = array(
    '#type' => 'value',
    '#value' => $active_nodeuri,
  );

  if (empty($form_state['values']['nodeuri'])) {
    $form_state['values']['nodeuri'] = $active_nodeuri;
  }

  $form['nodeuri'] = array(
    '#type' => 'item',
    '#title' => t('Node URI format'),
    '#description' => t('Configure the way the URI of a node is generated. If the generated URI is empty, the URI will be generated as the raw path of the node (which will always produce a URI).'),
  );
  // Populate choices
  foreach ($plugins as $plugin) {
    $name = $plugin['name'];
    $title = check_plain($plugin['title']);
    if (!empty($plugin['settings form'])) {
      // Add link to settings form
      $title .=  ' ' . l(t('(Settings)'), 'admin/build/simplerdf/nodeuri/' . $name, array('attributes' => array('class' => 'simplerdf-nodeuri-settings-link')));
    }
    $form['nodeuri'][$name] = array(
      '#type' => 'radio',
      '#title' => $title,
      '#description' => filter_xss($plugin['description']),
      '#attributes' => array('class' => 'simplerdf-nodeuri-button'),
      '#parents' => array('nodeuri'),
      '#default_value' => $form_state['values']['nodeuri'],
      '#return_value' => $name,
    );
  }

  // Preview interface
  $form['preview'] = array(
    '#type' => 'fieldset',
    '#title' => t('Preview node URI'),
  );
  $form['preview']['nid'] = array(
    '#type' => 'textfield',
    '#title' => t('Node ID'),
    '#description' => t('Enter a node ID to preview the URI for.'),
    '#autocomplete_path' => 'simplerdf/autocomplete/nid',
    '#default_value' => $form_state['values']['nid'],
  );
  if ($form_state['storage']['preview']) {
    $uri = simplerdf_generate_nodeuri($form_state['values']['nodeuri'], node_load($form_state['values']['nid']));
    $form['preview']['uri'] = array(
      '#type' => 'item',
      '#title' => t('Generated URI'),
      '#value' => !empty($uri) ? check_plain($uri) : ('<em>' . t('Empty') . '</em>'),
    );
  }
  $form['preview']['preview'] = array(
    '#type' => 'submit',
    '#value' => t('Preview URI'),
    '#validate' => array('simplerdf_admin_nodeuri_preview'),
  );

  $form['changed'] = array(
    '#value' => t('The settings on this form have been changed. Please save settings below to avoid losing changes.'),
    '#suffix' => '</div>',
  );
  if ($form_state['values']['__nodeuri'] && $form_state['values']['nodeuri'] != $form_state['values']['__nodeuri']) {
    $form['changed']['#prefix'] = '<div id="simplerdf-changed-message" class="changed">';
  }
  else {
    $form['changed']['#prefix'] = '<div id="simplerdf-changed-message">';
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );

  return $form;
}

/**
 * Node URI preview callback.
 */
function simplerdf_admin_nodeuri_preview($form, &$form_state) {
  if (empty($form_state['values']['nid']) || !node_load($form_state['values']['nid'])) {
    form_set_error('nid', t('Please enter a valid node ID to preview the node URI.'));
  }
  else {
    $form_state['storage']['preview'] = TRUE;
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * Node URI generator form submit callback.
 */
function simplerdf_admin_nodeuri_form_submit($form, $form_state) {
  $nodeuri = $form_state['values']['nodeuri'];
  variable_set('simplerdf_nodeuri_generator', $nodeuri);

  simplerdf_clear_cache('node');
  drupal_set_message(t('Node URI generator saved.'));
}

/**
 * Node URI settings form.
 */
function simplerdf_admin_nodeuri_settings_form($form_state, $plugin = NULL) {
  drupal_set_title(t('Configure %nodeuri settings', array('%nodeuri' => $plugin['title'])));

  // Cache plugin
  if ($plugin) {
    $form['__plugin'] = array(
      '#type' => 'value',
      '#value' => $plugin,
    );
  }
  elseif ($form_state['values']['__plugin']) {
    $plugin = $form_state['values']['__plugin'];
  }

  // Load plugin
  if (!empty($plugin) && !empty($plugin['file']) && file_exists($plugin_file = $plugin['path'] . '/' . $plugin['file'])) {
    _simplerdf_invoke_include($plugin_file);
  }

  // Prepare default values
  if (!isset($form_state['values'])) {
    $form_state['values'] = array();
  }
  $settings = variable_get('simplerdf_nodeuri_settings', array());
  if (!empty($settings) && is_array($settings[$plugin['name']])) {
    $form_state['values'] = array_merge($form_state['values'], $settings[$plugin['name']]);
  }
  $form_state['values'] = ((array) $form_state['values']) + $plugin['settings defaults'];

  // Fetch plugin settings form
  if (function_exists($callback = $plugin['settings form'])) {
    if (is_array($subform = $callback($form_state))) {
      $form = array_merge($form, $subform);
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );

  return $form;
}

/**
 * Node URI settings form validate callback.
 */
function simplerdf_admin_nodeuri_settings_form_validate(&$form, &$form_state) {
  // Load plugin
  $plugin = $form_state['values']['__plugin'];
  if (!empty($plugin['file']) && file_exists($plugin_file = $plugin['path'] . '/' . $plugin['file'])) {
    _simplerdf_invoke_include($plugin_file);
  }
  if (function_exists($callback = $plugin['settings form validate'])) {
    // Pass through to plugin settings validator
    $callback($form, $form_state);
  }
}

/**
 * Node URI settings form submit callback.
 */
function simplerdf_admin_nodeuri_settings_form_submit($form, &$form_state) {
  $plugin = $form_state['values']['__plugin'];
  $settings = (array) variable_get('simplerdf_nodeuri_settings', array());
  $settings[$plugin['name']] = _simplerdf_admin_nodeuri_process_values($plugin['settings defaults'], $form_state['values']);
  variable_set('simplerdf_nodeuri_settings', $settings);

  simplerdf_clear_cache_all();
  drupal_set_message(t('Settings for %nodeuri have been saved.', array('%nodeuri' => $plugin['title'])));
}

/**
 * Helper function to construct settings by recursively traversing defaults
 * and values arrays.
 *
 * @todo Test whether recursion works properly.
 *
 * @param array $defaults
 *   Default settings, used for detecting which values to save.
 * @param array $values
 *   Values received from the settings form.
 *
 * @return array
 *   Structured settings.
 */
function _simplerdf_admin_nodeuri_process_values($defaults, $values) {
  $settings = array();
  foreach ($defaults as $key => $value) {
    // Safety: skip objects
    if (is_object($value)) {
      continue;
    }
    // Recurse arrays
    elseif (is_array($value) && is_array($values[$key])) {
      $settings[$key] = _simplerdf_admin_nodeuri_process_values($value, $values[$key]);
    }
    else {
      $v = isset($values[$key]) ? $values[$key] : $value;
      // Value is set
      if (isset($values[$key])) {
        $v = $values[$key];
        // Default value is non-null
        if (isset($value)) {
          // Use type of default value
          settype($v, gettype($value));
        }
      }
      // Use default value
      else {
        $v = $value;
      }
      // Save settings
      $settings[$key] = $v;
    }
  }

  return $settings;
}

/**
 * Settings form.
 */
function simplerdf_admin_settings_form($form_state) {
  $form['view_path'] = array(
    '#type' => 'fieldset',
    '#title' => t('RDF view path'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['view_path']['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Node RDF path'),
    '#field_prefix' => 'node/%/',
    '#default_value' => variable_get('simplerdf_view_path', 'rdf'),
    '#required' => TRUE,
    '#description' => t('Internal path to display node RDF documents at.'),
  );

  $form['view_tab'] = array(
    '#type' => 'fieldset',
    '#title' => t('RDF view tab'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#description' => t('Configure tab settings.'),
  );
  $form['view_tab']['tab'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show tab'),
    '#default_value' => variable_get('simplerdf_view_tab', TRUE),
    '#description' => t('Display tab for viewing RDF documents.'),
  );
  $form['view_tab']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Tab title'),
    '#default_value' => variable_get('simplerdf_view_title', 'RDF'),
    '#required' => TRUE,
    '#description' => t('If the checkbox "Show tab" above is selected, set this to what should be displayed for the RDF tab label.'),
  );

  $form['format'] = array(
    '#type' => 'fieldset',
    '#title' => t('RDF format'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#description' => t('Configure the formats used by the ARC2 library to handle RDF.'),
  );
  module_load_include('inc', 'simplerdf', 'includes/simplerdf.arc');
  if (simplerdf_arc_include()) {
    $formats = array_map('_simplerdf_admin_get_friendly_format', simplerdf_arc_get_formats());
    ksort($formats);
    $form['format']['output_format'] = array(
      '#type' => 'radios',
      '#title' => t('Output format'),
      '#options' => $formats,
      '#default_value' => variable_get('simplerdf_format', SIMPLERDF_DEFAULT_FORMAT),
    );
  }
  else {
    $form['format']['error'] = array(
      '#type' => 'markup',
      '#value' => t('The ARC2 library is missing.'),
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );

  return $form;
}

/**
 * Map an ARC2 serializer format code name to a user-friendly name.
 *
 * @param string $format
 *   Serializer code name, such as NTriples.
 *
 * @return string
 *   User-friendly name, or original name if none matched.
 */
function _simplerdf_admin_get_friendly_format($format) {
  static $names;
  if (!$names) {
    $names = array(
      'NTriples' => t('N-Triples'),
      'POSHRDF' => t('POSH RDF'),
      'RDFJSON' => t('RDF/JSON'),
      'RDFXML' => t('RDF/XML'),
      'RSS10' => t('RSS 1.0'),
    );
  }
  return isset($names[$format]) ? $names[$format] : check_plain($format);
}

/**
 * Settings form validation callback.
 */
function simplerdf_admin_settings_form_validate($form, &$form_state) {
  extract($form_state['values']);
  if (preg_match('/[^A-Za-z0-9]/', $path)) {
    form_set_error('path', t('Invalid character detected for RDF view path. Please enter only lowercase, uppercase, and numeric characters.'));
  }
  elseif ($path != variable_get('simplerdf_view_path', 'rdf') && db_result(db_query("SELECT 1 FROM {menu_router} WHERE path = '%s'", "node/%/$path"))) {
    form_set_error('path', t('The specified path already exists. Please enter different one.'));
  }
}

/**
 * Settings form submit callback.
 */
function simplerdf_admin_settings_form_submit($form, &$form_state) {
  extract($form_state['values']);

  $rebuild = FALSE;
  if ($path != variable_get('simplerdf_view_path', 'rdf')) {
    variable_set('simplerdf_view_path', $path);
    $rebuild = TRUE;
  }
  if ($tab != variable_get('simplerdf_view_tab', TRUE)) {
    variable_set('simplerdf_view_tab', $tab);
    $rebuild = TRUE;
  }
  if ($title != variable_get('simplerdf_view_title', 'RDF')) {
    variable_set('simplerdf_view_title', $title);
    $rebuild = TRUE;
  }

  if ($rebuild) {
    // Rebuild menu
    $form_state['redirect'] = 'admin/build/simplerdf/settings/rebuild';
  }

  // Changed format
  if ($output_format != variable_get('simplerdf_format', SIMPLERDF_DEFAULT_FORMAT)) {
    variable_set('simplerdf_format', $output_format);
    // Clear just document cache
    // FIXME Execute custom query to account for colon
    cache_clear_all('%:%:document', 'cache_simplerdf', TRUE);
  }

  drupal_set_message(t('The settings have been saved.'));
}

/**
 * Rebuild menu for settings change.
 */
function _simplerdf_admin_rebuild() {
  menu_rebuild();
  drupal_set_message(t('The menu router has been rebuilt.'));
  drupal_goto('admin/build/simplerdf/settings');
}
