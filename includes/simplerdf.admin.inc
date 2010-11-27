<?php
// $Id$
/**
 * @file
 * Admin UI forms and callbacks.
 */

/**
 * @todo ???
 * Form for configuring node property mappings.
 */
function simplerdf_admin_mappings_form($form_state, $content_type = NULL) {
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
    '#value' => t('Enter one or more RDF property mapping (separated by commas) for the node class and each node field listed below. The fields are grouped together by their property mappers for clarity. Namespace prefixes defined for this site are listed in the table below (open the "RDF namespaces" fieldset to view). You may use these prefixes in the property mappings. Note: specified mappings will not be checked against their vocabularies.'),
    '#suffix' => '</p>',
  );
  $form['legend'] = array(
    '#type' => 'markup',
    '#value' => theme('simplerdf_namespaces'),
  );

  // Load mappings
  $rdf_mappings = simplerdf_get_mappings($content_type);

  // Build mappings
  $form['mappings'] = array();
  $fields = simplerdf_get_fields($content_type);
  foreach ($fields as $name => $field) {
    $mapping = array(
      'field' => check_plain($field['title']),
      'field_name' => check_plain($name),
      'properties' => array('#tree' => TRUE),
      'mapper' => $field['mapper'],
    );
    // Mapping textfield
    $mapping['properties'][$name] = array(
      '#type' => 'textfield',
      '#size' => 50,
      '#maxlength' => 100,
      '#default_value' => implode(', ', $rdf_mappings[$name]),
    );

    $form['mappings'][] = $mapping;
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Themes the Simple RDF mappings form as a table, using the theme hook
 * <code>simplerdf_mappings_table</code> for rendering.
 *
 * @param array $form
 *   The fully built form from simplerdf_admin_mappings_form().
 *
 * @return string
 *   Form with fully rendered mappings table.
 *
 * @ingroup themeable
 * @ingroup simplerdf_themeable
 */
function theme_simplerdf_admin_mappings_form($form) {
  ctools_include('plugins');
  $output = '';

  // Prepare mappers
  $plugins = ctools_get_plugins('simplerdf', 'mappers');
  $mappers = array();
  foreach ($plugins as $name => $plugin) {
    $mappers[$name] = check_plain($plugin['title']);
  }

  // Render form
  foreach ($form as $name => $elements) {
    // Mapping table
    if ($name == 'mappings') {
      foreach ($elements as &$mapping) {
        $mapping['properties'] = drupal_render($mapping['properties'][$mapping['field_name']]);
      }
      unset($mapping);
      $output .= theme('simplerdf_mappings_table', $elements, $mappers);
    }
    // Default form element
    else {
      $output .= drupal_render($elements);
    }
  }

  return $output;
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
    $mapping = (object) compact('content_type', 'field', 'properties');
    drupal_write_record('simplerdf_mappings', $mapping);
  }

  // Clear mapping cache
  simplerdf_clear_cache('mappings', $content_type);

  drupal_set_message(t('RDF mappings have been updated.'));
}

/**
 * @todo Admin dashboard
 * Dashboard page.
 */
function simplerdf_admin_dashboard() {
  return '';
}

/**
 * Node URI generator choice form.
 */
function simplerdf_admin_nodeuri_form($form_state) {
  $path = drupal_get_path('module', 'simplerdf');
  drupal_add_css($path . '/simplerdf.admin.css');
  drupal_add_js($path . '/simplerdf.admin.js');

  $plugins = ctools_get_plugins('simplerdf', 'nodeuri_generators');
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
    '#description' => t('Configure the way the URI of a node is generated.'),
  );
  // Populate choices
  foreach ($plugins as $name => $plugin) {
    $title = check_plain($plugin['title']);
    if (function_exists($plugin['settings form'])) {
      // Add link to settings form
      $title .=  ' ' . l(t('(Settings)'), 'admin/build/simplerdf/nodeuri/' . $name);
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
      '#value' => check_plain($uri),
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

  // Prepare default values
  $form_state['values'] = ((array) $form_state['values']) + $plugin['settings defaults'];

  // Fetch plugin settings form
  if (function_exists($callback = $plugin['settings form'])) {
    $form = array_merge($form, $callback($form_state));
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );
}

/**
 * Node URI settings form validate callback.
 */
function simplerdf_admin_nodeuri_settings_form_validate(&$form, &$form_state) {
  $plugin = $form_state['values']['__plugin'];
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
  drupal_set_message(t('Settings for %nodeuri have been saved.', array('%nodeuri' => $plugin['title'])));
}

/**
 * Helper function to construct settings by recursively traversing defaults
 * and values arrays.
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
  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('RDF view path'),
    '#prefix' => 'node/%/',
    '#default_value' => variable_get('simplerdf_view_path', 'rdf'),
    '#description' => t('Internal path to display node RDF documents at.'),
  );

  $form['show_tab'] = array(
    '#type' => 'fieldset',
    '#title' => t('RDF view tab'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#description' => t('Configure tab settings.'),
  );
  $form['tab'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show tab'),
    '#default_value' => variable_get('simplerdf_view_tab', TRUE),
    '#description' => t('Display tab for viewing node RDF document pages.'),
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Tab title'),
    '#default_value' => variable_get('simplerdf_view_title', 'RDF'),
    '#description' => t('If "RDF view tab" is selected, set this to what should be displayed for the RDF tab label.'),
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
      'LegacyHTML' => t('Legacy HTML'),
      'LegacyJSON' => t('Legacy JSON'),
      'LegacyXML' => t('Legacy XML'),
      'NTriples' => t('N-Triples'),
      'POSHRDF' => t('POSH RDF'),
      'RDFJSON' => t('RDF/JSON'),
      'RDFXML' => t('RDF/XML'),
      'RSS10' => t('RSS 1.0'),
    );
  }
  return isset($names[$format]) ? $names[$format] : $format;
}

/**
 * Settings form validation callback.
 */
function simplerdf_admin_settings_form_validate($form, &$form_state) {
  extract($form_state['values']);
  if (preg_match('/[^A-Za-z0-9]/', $path)) {
    form_set_error('path', t('Invalid character detected for RDF view path. Please enter only lowercase, uppercase, and numeric characters.'));
  }
  elseif (db_result(db_query("SELECT 1 FROM {menu_router} WHERE path = '%s'", "node/%/$path"))) {
    form_set_error('path', t('The specified path already exists. Please enter different one.'));
  }
}

/**
 * Settings form submit callback.
 */
function simplerdf_admin_settings_form_submit($form, &$form_state) {
  extract($form_state['values']);
  variable_set('simplerdf_view_path', $path);
  variable_set('simplerdf_view_tab', $tab);
  variable_set('simplerdf_view_title', $title);

  // Changed format
  if ($output_format != variable_get('simplerdf_format', SIMPLERDF_DEFAULT_FORMAT)) {
    variable_set('simplerdf_format', $output_format);
    // Clear just document cache
    cache_clear_all('node:document:', 'cache_simplerdf', TRUE);
  }
}
