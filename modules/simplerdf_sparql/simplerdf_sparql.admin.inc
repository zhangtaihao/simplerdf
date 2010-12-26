<?php
// $Id$
/**
 * @file
 * Administration interface.
 */

/**
 * Settings form.
 */
function simplerdf_sparql_admin_form($form_state) {
  // Re-index button
  $form['reindex'] = array(
    '#type' => 'fieldset',
    '#title' => t('Re-index'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#description' => t('Rebuild the RDF database by re-indexing all site objects.'),
  );
  $form['reindex']['reindex'] = array(
    '#type' => 'submit',
    '#value' => t('Re-index RDF data'),
    '#submit' => array('simplerdf_sparql_admin_settings_rebuild'),
  );

  // Endpoint settings
  $form['endpoint'] = array(
    '#type' => 'fieldset',
    '#title' => 'SPARQL endpoint settings',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['endpoint']['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Endpoint path'),
    '#required' => TRUE,
    '#field_prefix' => url(NULL, array('absolute' => TRUE)),
    '#default_value' => variable_get('simplerdf_sparql_path', 'simplerdf/sparql'),
    '#description' => t('The SPARQL endpoint will be available under this path. Please enter a valid Drupal internal path delimited by forward slashes (/). Each path part must contain one or more alphanumeric characters (A-Z, a-z, 0-9), hyphens (-), or underscores (_).'),
  );
  $form['endpoint']['menu_type'] = array(
    '#type' => 'radios',
    '#title' => t('Menu item type'),
    '#required' => TRUE,
    '#options' => array(
      MENU_CALLBACK => t('Callback (no displayable item)'),
      MENU_NORMAL_ITEM => t('Menu item'),
      MENU_LOCAL_TASK => t('Page tab'),
    ),
    '#default_value' => variable_get('simplerdf_sparql_menu_type', MENU_CALLBACK),
    '#description' => t('Page paths in Drupal are achieved using a menu router item. Please select the menu router item type to designate the above path as.'),
  );
  $form['endpoint']['read_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Endpoint read API key'),
    '#default_value' => variable_get('simplerdf_sparql_arc2_read_key', NULL),
    '#description' => t('The read key is passed through to ARC2 for basic authorization for regular SPARQL queries.'),
  );
  $form['endpoint']['write_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Endpoint write API key (for SPARQL Update)'),
    '#default_value' => variable_get('simplerdf_sparql_arc2_write_key', NULL),
    '#description' => t('The write key is passed through to ARC2 for basic authorization to execute SPARQL update queries.'),
  );
  $form['endpoint']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
    '#validate' => array('simplerdf_sparql_admin_settings_validate'),
    '#submit' => array('simplerdf_sparql_admin_settings_submit'),
  );

  return $form;
}

/**
 * Rebuild SPARQL data.
 */
function simplerdf_sparql_admin_settings_rebuild($form, &$form_state) {
  simplerdf_sparql_reset();
  if (simplerdf_sparql_index()) {
    drupal_set_message(t('All site objects have been re-indexed.'));
  }
  else {
    drupal_set_message(t('Could not obtain lock to re-index objects.'), 'warning');
  }
}

/**
 * Settings form validator.
 */
function simplerdf_sparql_admin_settings_validate($form, &$form_state) {
  $values = $form_state['values'];

  // Validate path
  $path = $values['path'];
  if (!preg_match('`^[a-z0-9_-]+(/[a-z0-9_-]+)*$`i', $path)) {
    form_set_error('path', t('The entered path contains invalid characters and/or patterns.'));
  }
  elseif ($path != variable_get('simplerdf_sparql_path', 'simplerdf/sparql') && db_result(db_query("SELECT 1 FROM {menu_router} WHERE path = '%s'", $path))) {
    form_set_error('path', t('The specified path already exists. Please enter different one.'));
  }
}

/**
 * Settings form submit handler.
 */
function simplerdf_sparql_admin_settings_submit($form, &$form_state) {
  $values = $form_state['values'];
  variable_set('simplerdf_sparql_path', $values['path']);
  variable_set('simplerdf_sparql_menu_type', $values['menu_type']);
  variable_set('simplerdf_sparql_arc2_read_key', $values['read_key']);
  variable_set('simplerdf_sparql_arc2_write_key', $values['write_key']);
  drupal_set_message(t('SPARQL settings have been saved.'));
}
