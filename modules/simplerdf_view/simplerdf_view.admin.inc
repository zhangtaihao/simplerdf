<?php
// $Id$
/**
 * @file
 * Administration interface.
 */

/**
 * Settings form.
 */
function simplerdf_view_admin_settings_form($form_state) {
  $form['path'] = array(
    '#type' => 'fieldset',
    '#title' => t('RDF view path'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['path']['node_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Node RDF path'),
    '#field_prefix' => 'node/%/',
    '#default_value' => variable_get('simplerdf_view_node_path', SIMPLERDF_VIEW_DEFAULT_PATH),
    '#required' => TRUE,
    '#description' => t('Internal path to display node RDF documents at.'),
  );
  $form['path']['node_override_rdf'] = array(
    '#type' => 'checkbox',
    '#title' => t('Override node RDF output'),
    '#default_value' => variable_get('simplerdf_view_node_override_rdf', FALSE),
    '#description' => t('Check this option to take over RDF display under "node/%/rdf". NOTE: Simple RDF does not include extra RDF resources defined by modules implementing the RDF API.'),
  );
  $form['path']['user_path'] = array(
    '#type' => 'textfield',
    '#title' => t('User RDF path'),
    '#field_prefix' => 'user/%/',
    '#default_value' => variable_get('simplerdf_view_user_path', 'rdf'),
    '#required' => TRUE,
    '#description' => t('Internal path to display user RDF documents at.'),
  );
  $form['path']['user_override_rdf'] = array(
    '#type' => 'checkbox',
    '#title' => t('Override user RDF output'),
    '#default_value' => variable_get('simplerdf_view_user_override_rdf', FALSE),
    '#description' => t('Check this option to take over RDF display under "user/%/rdf". NOTE: Simple RDF does not include extra RDF resources defined by modules implementing the RDF API.'),
  );
  if (module_exists('simplerdf_taxonomy')) {
    $form['path']['term_path'] = array(
      '#type' => 'textfield',
      '#title' => t('Term RDF path'),
      '#field_prefix' => 'taxonomy/term/%/',
      '#default_value' => variable_get('simplerdf_view_term_path', 'rdf'),
      '#required' => TRUE,
      '#description' => t('Internal path to display term RDF documents at.'),
    );
  }

  $form['tab'] = array(
    '#type' => 'fieldset',
    '#title' => t('RDF view tab'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#description' => t('Configure tab settings.'),
  );
  $form['tab']['tab'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show tab'),
    '#default_value' => variable_get('simplerdf_view_tab', TRUE),
    '#description' => t('Display tab for viewing RDF documents.'),
  );
  $form['tab']['title'] = array(
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
  if (simplerdf_view_arc_include()) {
    $formats = array_map('_simplerdf_view_admin_get_friendly_format', simplerdf_view_arc_get_formats());
    ksort($formats);
    $form['format']['output_format'] = array(
      '#type' => 'radios',
      '#title' => t('Output format'),
      '#options' => $formats,
      '#default_value' => variable_get('simplerdf_view_format', SIMPLERDF_VIEW_DEFAULT_FORMAT),
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
function _simplerdf_view_admin_get_friendly_format($format) {
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
function simplerdf_view_admin_settings_form_validate($form, &$form_state) {
  extract($form_state['values']);
  $rdf_exists = module_exists('rdf');

  if (preg_match('/[^A-Za-z0-9]/', $node_path)) {
    form_set_error('node_path', t('Invalid character detected for RDF view path. Please enter only lowercase, uppercase, and numeric characters.'));
  }
  else {
    $path_changed = $node_path != variable_get('simplerdf_view_node_path', SIMPLERDF_VIEW_DEFAULT_PATH);
    if (($path_changed && !($node_override_rdf && $node_path == 'rdf') ||
          !$path_changed && !$node_override_rdf && $node_path == 'rdf' && $rdf_exists) &&
        db_result(db_query("SELECT 1 FROM {menu_router} WHERE path = '%s'", "node/%/$node_path"))) {
      form_set_error('node_path', t('The specified path already exists. Please enter different one.'));
    }
  }

  if (preg_match('/[^A-Za-z0-9]/', $user_path)) {
    form_set_error('user_path', t('Invalid character detected for user RDF path. Please enter only lowercase, uppercase, and numeric characters.'));
  }
  else {
    $path_changed = $user_path != variable_get('simplerdf_view_user_path', SIMPLERDF_VIEW_DEFAULT_PATH);
    if (($path_changed && !($user_override_rdf && $user_path == 'rdf') ||
          !$path_changed && !$user_override_rdf && $user_path == 'rdf' && $rdf_exists) &&
        db_result(db_query("SELECT 1 FROM {menu_router} WHERE path = '%s'", "user/%/$user_path"))) {
      form_set_error('user_path', t('The specified path already exists. Please enter different one.'));
    }
  }

  if (isset($term_path)) {
    if (preg_match('/[^A-Za-z0-9]/', $term_path)) {
      form_set_error('term_path', t('Invalid character detected for term RDF path. Please enter only lowercase, uppercase, and numeric characters.'));
    }
    elseif ($term_path != variable_get('simplerdf_view_term_path', SIMPLERDF_VIEW_DEFAULT_PATH) && db_result(db_query("SELECT 1 FROM {menu_router} WHERE path = '%s'", "taxonomy/term/%/$term_path"))) {
      form_set_error('term_path', t('The specified path already exists. Please enter different one.'));
    }
  }
}

/**
 * Settings form submit callback.
 */
function simplerdf_view_admin_settings_form_submit($form, &$form_state) {
  extract($form_state['values']);

  $rebuild = FALSE;
  if ($node_path != variable_get('simplerdf_view_node_path', SIMPLERDF_VIEW_DEFAULT_PATH)) {
    variable_set('simplerdf_view_node_path', $node_path);
    $rebuild = TRUE;
  }
  if ($node_override_rdf != variable_get('simplerdf_view_node_override_rdf', FALSE)) {
    variable_set('simplerdf_view_node_override_rdf', $node_override_rdf);
    $rebuild = TRUE;
  }
  if ($user_path != variable_get('simplerdf_view_user_path', SIMPLERDF_VIEW_DEFAULT_PATH)) {
    variable_set('simplerdf_view_user_path', $user_path);
    $rebuild = TRUE;
  }
  if ($user_override_rdf != variable_get('simplerdf_view_user_override_rdf', FALSE)) {
    variable_set('simplerdf_view_user_override_rdf', $user_override_rdf);
    $rebuild = TRUE;
  }
  if (isset($term_path)) {
    if ($term_path != variable_get('simplerdf_view_term_path', SIMPLERDF_VIEW_DEFAULT_PATH)) {
      variable_set('simplerdf_view_term_path', $term_path);
      $rebuild = TRUE;
    }
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
    $form_state['redirect'] = 'admin/build/simplerdf/view/rebuild';
  }

  // Changed format
  if ($output_format != variable_get('simplerdf_view_format', SIMPLERDF_VIEW_DEFAULT_FORMAT)) {
    variable_set('simplerdf_view_format', $output_format);
    // Clear just document cache
    // FIXME Execute custom query to account for colon
    cache_clear_all('%:%:document', 'cache_simplerdf', TRUE);
  }

  drupal_set_message(t('The settings have been saved.'));
}

/**
 * Rebuild menu for settings change.
 */
function _simplerdf_view_admin_rebuild() {
  menu_rebuild();
  drupal_set_message(t('The menu router has been rebuilt.'));
  drupal_goto('admin/build/simplerdf/view');
}
