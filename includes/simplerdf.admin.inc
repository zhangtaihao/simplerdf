<?php
// $Id$
/**
 * @file
 * Admin UI forms and callbacks.
 */


/************************************************
 * Mapping forms.
 ************************************************/

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
  // Prepare mappers
  $plugins = simplerdf_get_plugins('mappers');
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
 * Form for configuring node property mappings.
 */
function simplerdf_admin_node_mappings_form($form_state, $content_type) {
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
  $rdf_mappings = simplerdf_get_mappings('node', $content_type);

  // Build mappings
  $form['mappings'] = array(
    '#theme' => 'simplerdf_admin_mappings_table',
  );
  $fields = simplerdf_get_fields('node', $content_type);
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
 * Node mapping form submit callback.
 */
function simplerdf_admin_node_mappings_form_submit($form, $form_state) {
  $values = $form_state['values'];
  $content_type = $values['__content_type'];

  $mappings = array();

  // Add mappings
  foreach ($values['properties'] as $field => $properties) {
    // Normalize RDF properties
    $properties = array_filter(array_map('trim', preg_split('/\s*,\s*/', $properties)));
    if (!empty($properties)) {
      $mappings[$field] = $properties;
    }
  }

  // Save
  simplerdf_save_mappings('node', $content_type, $mappings);

  // Clear cache
  simplerdf_clear_cache('mappings', 'node', $content_type);
  simplerdf_clear_cache('node');

  drupal_set_message(t('RDF mappings have been updated.'));
}

/**
 * User mappings form.
 */
function simplerdf_admin_user_mappings_form($form_state) {
  drupal_set_title(t('Manage user RDF mappings'));

  $form['help'] = array(
    '#type' => 'markup',
    '#prefix' => '<p>',
    '#value' => t('Enter one or more RDF property mapping (separated by commas) for each user field listed below. Namespace prefixes defined for this site are listed in the table below (open the "RDF namespaces" fieldset to view). You may use these prefixes in the property mappings. Note: specified mappings will not be checked against their vocabularies.'),
    '#suffix' => '</p>',
  );
  $form['legend'] = array(
    '#type' => 'markup',
    '#value' => theme('simplerdf_namespaces'),
  );

  // Load mappings
  $mappings = simplerdf_get_mappings('user');

  // Build mappings
  $form['mappings'] = array(
    '#theme' => 'simplerdf_admin_basic_mappings_table',
  );
  $fields = simplerdf_get_fields('user');
  foreach ($fields as $name => $field) {
    $mapping = array(
      '#field_title' => $field['title'],
      '#field_name' => $name,
      'properties' => array('#tree' => TRUE),
    );
    // Mapping textfield
    $mapping['properties'][$name] = array(
      '#type' => 'textfield',
      '#size' => 20,
      '#maxlength' => 100,
      '#default_value' => implode(', ', $mappings[$name]),
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
 * Mappings form submit handler.
 */
function simplerdf_admin_user_mappings_form_submit(&$form, &$form_state) {
  $values = $form_state['values'];

  $mappings = array();
  $fields = simplerdf_get_fields('user');
  foreach (array_keys($fields) as $name) {
    if (!empty($values['properties'][$name])) {
      $properties = array_filter(array_map('trim', preg_split('/\s*,\s*/', $values['properties'][$name])));
      if (!empty($properties)) {
        $mappings[$name] = $properties;
      }
    }
  }
  simplerdf_save_mappings('user', NULL, $mappings);
  simplerdf_clear_cache('mappings', 'user');
  simplerdf_clear_cache('user');

  drupal_set_message(t('RDF mappings have been updated.'));
}


/************************************************
 * General administration UI.
 ************************************************/

/**
 * Administration dashboard page.
 */
function simplerdf_admin_dashboard() {
  $path = drupal_get_path('module', 'simplerdf');
  drupal_add_css($path . '/simplerdf.admin.css');

  $output = '<div class="simplerdf-admin-dashboard">';

  $output .= '<p>' . t('An overview of the Simple RDF configuration for this site is displayed on this page.');

  $output .= '<h2>' . t('Basic operations') . '</h2>';
  $output .= '<p>' . l(t('Flush Simple RDF cache'), 'admin/build/simplerdf/flush') . t(' (clear cached RDF documents and other temporary data)') . '</p>';
  $output .= '<p>' . l(t('Re-index site objects'), 'admin/build/simplerdf/relindex') . t(' (rebuild auxiliary data used by Simple RDF to track site objects)') . '</p>';

  // Display namespaces
  $output .= '<h2>' . t('RDF namespaces') . '</h2>';

  $output .= theme('simplerdf_namespaces_table');
  if (module_exists('simplerdf_namespaces')) {
    $output .= '<div class="simplerdf-admin-links">';
    $output .= l(t('Manage custom namespaces'), 'admin/build/simplerdf/namespaces');
    $output .= '</div>';
  }
/*
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
*/
  // Active plugins
  $output .= '<h2>' . t('Active plugins') . '</h2>';

  $header = array(
    t('Type'),
    t('Plugins'),
  );
  $type_names = array(
    'uri_formats' => t('URI formats'),
    'mappers' => t('Field mappers'),
    'content_mappers' => t('Content field mappers'),
  );
  $rows = array();
  foreach ($type_names as $type => $type_name) {
    $plugin_titles = array();
    $plugins = simplerdf_get_plugins($type);
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
 * Re-index object relations.
 */
function simplerdf_admin_rel_index() {
  simplerdf_rel_index();
  drupal_set_message(t('Objects have been re-indexed.'));
  drupal_goto('admin/build/simplerdf');
}

/**
 * Object URI configuration overview.
 */
function simplerdf_admin_uri_overview() {
  $output = '<p>' . t('The active URI format for each object type is listed in the table below. Click on "configure" to change settings for that type. If any format below is "None", please select one in order for mapping RDF for the object type to function. NOTE: if the RDF module is used to display RDF documents, only the default aliased format is applicable for node and user objects as RDF will not recognize any path other than its default canonical URL.') . '</p>';

  // Build table
  $header = array(
    t('Type'),
    t('Format'),
    t('Operations'),
  );

  $rows = array();
  $types = simplerdf_get_types();
  foreach ($types as $type => $type_title) {
    // Type name
    $row = array($type_title);
    // Type URI format; trigger default format detection via API
    $plugin = simplerdf_get_uri_format_plugin($type);
    if ($plugin) {
      $row[] = filter_xss($plugin['title']);
    }
    else {
      $row[] = t('%none', array('%none' => t('None')));
    }
    // Operations
    $row[] = l(t('configure'), "admin/build/simplerdf/uri/$type");

    $rows[] = $row;
  }

  $output .= theme('table', $header, $rows);
  return $output;
}

/**
 * Object URI format choice form.
 */
function simplerdf_admin_uri_form(&$form_state, $type) {
  $path = drupal_get_path('module', 'simplerdf');
  drupal_add_css($path . '/simplerdf.admin.css');
  drupal_add_js($path . '/simplerdf.admin.js');

  // Process object type
  $info = _simplerdf_type_info();
  $types = $info['types'];
  $map = $info['map'];
  if ($type && array_key_exists($type, $types)) {
    // Form entry
    $form['__type'] = array(
      '#type' => 'value',
      '#value' => $type,
    );
  }
  else {
    // Invalid type
    drupal_not_found();
    exit;
  }
  $module = $map[$type];

  $plugins = simplerdf_get_plugins('uri_formats');
  $active_plugin = simplerdf_get_uri_format_plugin($type);
  $format = $active_plugin['name'];
  $all_settings = (array) variable_get('simplerdf_uri_settings', array());
  $type_settings = (array) $all_settings[$type];

  // Cache old setting
  $form['__format'] = array(
    '#type' => 'value',
    '#value' => $format,
  );

  if (empty($form_state['values']['format'])) {
    $form_state['values']['format'] = $format;
  }

  $form['format'] = array(
    '#type' => 'item',
    '#title' => t('URI format'),
    '#description' => t('Select the format of the URI of an object. Preview it to make sure it works properly.'),
  );
  // Populate choices
  foreach ($plugins as $plugin) {
    // Skip incompatible plugins
    if ($plugin['object types'] && !in_array($type, $plugin['object types'])) {
      continue;
    }

    $name = $plugin['name'];
    $title = check_plain($plugin['title']);
    if ($plugin['aliasable'] || !empty($plugin['settings form'])) {
      if ($plugin['aliasable'] && $type_settings[$name]['alias']) {
        // Append alias status of format
        $title .= t(' (aliased)');
      }
      // Add link to settings form
      $title .=  ' ' . l(t('[configure]'), "admin/build/simplerdf/uri/$type/$name", array('attributes' => array('class' => 'simplerdf-nodeuri-settings-link')));
    }
    $form['format'][$name] = array(
      '#type' => 'radio',
      '#title' => $title,
      '#description' => filter_xss($plugin['description']),
      '#attributes' => array('class' => 'simplerdf-uri-button'),
      '#parents' => array('format'),
      '#default_value' => $form_state['values']['format'],
      '#return_value' => $name,
    );
  }

  // Preview interface
  $form['preview'] = array(
    '#type' => 'fieldset',
    '#title' => t('Preview URI'),
  );
  $form['preview']['id'] = array(
    '#type' => 'textfield',
    '#title' => t('@type ID', array('@type' => $types[$type])),
    '#description' => t('Enter a ID to preview the URI for.'),
    '#default_value' => $form_state['values']['id'],
  );
  // Check whether type supports autocomplete
  if (module_hook($module, 'simplerdf_' . $type . '_autocomplete')) {
    $form['preview']['id']['#autocomplete_path'] = 'simplerdf/autocomplete/preview_id/' . $type;
  }

  if ($form_state['storage']['preview']) {
    unset($form_state['storage']['preview']);
    // Format URI and display
    $id = $form_state['values']['id'];
    $object = simplerdf_load($type, $id);
    if ($object) {
      $uri = simplerdf_format_uri($type, $id, $object, $form_state['values']['format']);
    }
    $form['preview']['uri'] = array(
      '#type' => 'item',
      '#title' => t('Generated URI'),
      '#value' => !empty($uri) ? check_plain($uri) : ('<em>' . t('Empty') . '</em>'),
    );
  }
  $form['preview']['preview'] = array(
    '#type' => 'submit',
    '#value' => t('Preview URI'),
    '#validate' => array('simplerdf_admin_uri_preview'),
  );

  $form['changed'] = array(
    '#value' => t('The settings on this form have been changed. Please save settings below to avoid losing changes.'),
    '#suffix' => '</div>',
  );
  if ($form_state['values']['__format'] && $form_state['values']['format'] != $form_state['values']['__format']) {
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
 * URI preview callback.
 */
function simplerdf_admin_uri_preview($form, &$form_state) {
  if (empty($form_state['values']['id']) || !simplerdf_load($form_state['values']['__type'], $form_state['values']['id'])) {
    form_set_error('id', t('Please enter a valid ID to preview the URI.'));
  }
  else {
    $form_state['storage']['preview'] = TRUE;
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * URI format form submit callback.
 */
function simplerdf_admin_uri_form_submit($form, &$form_state) {
  $type = $form_state['values']['__type'];
  $format = $form_state['values']['format'];
  $formats = (array) variable_get('simplerdf_uri_formats', array());
  $formats[$type] = $format;
  variable_set('simplerdf_uri_formats', $formats);

  simplerdf_clear_cache($type);
  $types = simplerdf_get_types();
  drupal_set_message(t('@type URI format saved.', array('@type' => $types[$type])));

  $form_state['redirect'] = 'admin/build/simplerdf/uri';
}

/**
 * URI settings form.
 */
function simplerdf_admin_uri_settings_form($form_state, $type, $format) {
  $types = simplerdf_get_types();
  $plugin = simplerdf_get_plugins('uri_formats', $format);
  // Sanity checks
  if ($types[$type] && $plugin && (!$plugin['object types'] || in_array($type, $plugin['object types'])) && ($plugin['aliasable'] || $plugin['settings form'])) {
    $form['__type'] = array(
      '#type' => 'value',
      '#value' => $type,
    );
    // Cache plugin
    $form['__plugin'] = array(
      '#type' => 'value',
      '#value' => $plugin,
    );
  }
  else {
    drupal_not_found();
    exit;
  }

  drupal_set_title(t('Configure %format settings for %type', array('%format' => $plugin['title'], '%type' => $types[$type])));

  // Load plugin
  if (!empty($plugin) && !empty($plugin['file']) && file_exists($plugin_file = $plugin['path'] . '/' . $plugin['file'])) {
    _simplerdf_invoke_include($plugin_file);
  }

  // Prepare default values
  if (!isset($form_state['values'])) {
    $form_state['values'] = array();
  }
  $all_settings = (array) variable_get('simplerdf_uri_settings', array());
  $config = (array) $all_settings[$type][$format];
  if (!isset($config['settings'])) {
    $config['settings'] = array();
  }
  if ($plugin['aliasable']) {
    $form_state['values']['alias'] = $config['alias'];
  }
  $form_state['values']['settings'] = $config['settings'] + $plugin['settings defaults'];

  // Alias setting
  if ($plugin['aliasable']) {
    $form['alias'] = array(
      '#type' => 'checkbox',
      '#title' => t('Aliased'),
      '#default_value' => $form_state['values']['alias'],
      '#description' => t('Check this option if you want Drupal to look up the resulting URI from this format for a path alias.'),
    );
  }

  // Fetch plugin settings form
  $form['settings'] = array(
    '#tree' => TRUE,
  );
  if (function_exists($callback = $plugin['settings form'])) {
    if (is_array($subform = $callback($form_state, $type))) {
      $form['settings'] = array_merge($form['settings'], $subform);
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );

  return $form;
}

/**
 * URI settings form validate callback.
 */
function simplerdf_admin_uri_settings_form_validate(&$form, &$form_state) {
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
 * URI settings form submit callback.
 */
function simplerdf_admin_uri_settings_form_submit($form, &$form_state) {
  $type = $form_state['values']['__type'];
  $plugin = $form_state['values']['__plugin'];
  $all_settings = (array) variable_get('simplerdf_uri_settings', array());
  $all_settings[$type][$plugin['name']]['alias'] = $form_state['values']['alias'];
  $all_settings[$type][$plugin['name']]['settings'] = _simplerdf_admin_uri_process_values($plugin['settings defaults'], $form_state['values']['settings']);
  variable_set('simplerdf_uri_settings', $all_settings);

  simplerdf_clear_cache_all();
  drupal_set_message(t('Settings for %format have been saved.', array('%format' => $plugin['title'])));

  $form_state['redirect'] = "admin/build/simplerdf/uri/$type";
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
function _simplerdf_admin_uri_process_values($defaults, $values) {
  $settings = array();
  foreach ($defaults as $key => $value) {
    // Safety: skip objects
    if (is_object($value)) {
      continue;
    }
    // Recurse arrays
    elseif (is_array($value) && is_array($values[$key])) {
      $settings[$key] = _simplerdf_admin_uri_process_values($value, $values[$key]);
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
