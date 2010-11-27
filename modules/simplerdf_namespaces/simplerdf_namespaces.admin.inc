<?php
// $Id$
/**
 * @file
 * Administration interfaces.
 */

/** Main administration interface path. */
define('SIMPLERDF_NAMESPACES_ADMIN_PATH', 'admin/build/simplerdf/namespaces');

/**
 * Namespace administration page.
 */
function simplerdf_namespaces_admin_list() {
  $output = '';

  // Custom namespaces edit table
  $output .= '<h2>' . t('Custom namespaces') . '</h2>';

  $header = array(
    t('Machine name'),
    t('Prefix'),
    t('Namespace'),
    t('State'),
    t('Operations'),
  );

  $states = array(
    'normal' => t('Normal'),
    'default' => t('Default'),
    'overridden' => t('Overridden'),
  );

  // Build namespace rows
  $rows = array();
  foreach (simplerdf_namespaces_load() as $name => $ns) {
    // Build object state
    $ns->state = _simplerdf_namespaces_detect_state($ns);

    // Build operations
    $ops = array(
      'edit' => NULL,
      'delete' => NULL,
    );

    if ($ns->state == 'default') {
      $ops['edit'] = t('Override');
    }
    else {
      $ops['edit'] = t('Edit');
    }
    if ($ns->state == 'normal') {
      $ops['delete'] = t('Delete');
    }
    elseif ($ns->state == 'overridden') {
      $ops['delete'] = t('Revert');
    }

    foreach ($ops as $op_name => $op) {
      $ops[$op_name] = l($op, SIMPLERDF_NAMESPACES_ADMIN_PATH . '/edit/' . $ns->prefix, array('class' => $op_name . '-link'));
    }

    // Build row
    $rows[] = array(
      check_plain($ns->name),
      check_plain($ns->prefix),
      check_plain($ns->uri),
      $states[$ns->state],
      array(
        'data' => implode(' | ', $ops),
        'class' => 'simplerdf-namespace-operations',
      ),
    );
  }

  // Render
  $output .= theme('table', $header, $rows, array('class' => 'simplerdf-namespaces-admin-list'));

  return $output;
}

/**
 * Namespace edit form.
 */
function simplerdf_namespaces_admin_edit_form($form_state, $ns = NULL) {
  $form = array();

  if ($ns) {
    drupal_set_title(t('Edit namespace %ns', array('%ns' => $ns->name)));

    // Cache old value
    $form['__ns'] = array(
      '#type' => 'value',
      '#value' => $ns,
    );
  }

  // Machine name
  $form['name'] = array(
    '#title' => t('Machine name'),
    '#description' => t('Unique code name of this namespace. Must contain only lowercase letters (a-z), numbers (0-9), and underscores (_). Once this value has been set, it cannot be changed.'),
  );
  if ($ns) {
    // Existing item
    $form['name'] += array(
      '#type' => 'item',
    );
  }
  else {
    // New item
    $form['name'] += array(
      '#type' => 'textfield',
      '#maxlength' => 64,
      '#size' => 30,
      '#required' => TRUE,
    );
  }

  $form['prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace prefix'),
    '#size' => 20,
    '#maxlength' => 64,
    '#required' => TRUE,
    '#description' => t('Namespace prefix as used in some RDF document formats. Do not include a colon (:) at the end.'),
  );

  $form['uri'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace name (URI)'),
    '#size' => 80,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#description' => t('Namespace URI indicating the base of objects grouped under the namespace.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Namespace edit form validation callback.
 */
function simplerdf_namespaces_admin_edit_form_validate(&$form, &$form_state) {
  $ns = (array) $form_state['values']['__ns'];
  $name = $form_state['values']['name'];
  $prefix = $form_state['values']['prefix'];
  $uri = $form_state['values']['uri'];

  // Prepare existing dataset
  $namespaces = module_invoke_all('rdf_namespaces');
  $names = array_keys(simplerdf_namespaces_load());

  // Machine name
  if (!$ns) {
    if (preg_match('/[^_a-z0-9]/', $name)) {
      form_set_error('name', t('Machine name contains invalid characters. Please enter only lowercase characters, numbers, and underscores.'));
    }
    elseif (in_array($name, $names)) {
      form_set_error('name', t('Machine name already exists.'));
    }
  }

  // Prefix
  if (!preg_match('/^[_A-Za-z][-._A-Za-z0-9]*$/', $prefix)) {
    form_set_error('prefix', t('Namespace prefix contains invalid characters. Please specify using XML-compliant format.'));
  }
  elseif ((empty($ns['prefix']) || $prefix != $ns['prefix']) && array_key_exists($prefix, $namespaces)) {
    form_set_error('prefix', t('Namespace prefix is already in use.'));
  }

  // URI
  if (!preg_match('`(/|#)$`', $uri)) {
    form_set_error('uri', t('The URI must end in "/" or "#" (without quotes).'));
  }
  elseif ((empty($ns['uri']) || $uri != $ns['uri']) && in_array($uri, $namespaces)) {
    form_set_error('uri', t('Namespace name is already defined.'));
  }
}

/**
 * Namespace edit form validate callback.
 */
function simplerdf_namespaces_admin_edit_form_submit($form, &$form_state) {
  $ns = new stdClass();
  $ns->name = $form_state['values']['__ns'] ? $form_state['values']['__ns']->name : $form_state['values']['name'];
  $ns->prefix = $form_state['values']['prefix'];
  $ns->uri = $form_state['values']['uri'];

  $saved = FALSE;
  if ($form_state['values']['__ns']) {
    $saved = drupal_write_record('simplerdf_namespaces', $ns, 'name');
  }
  else {
    $saved = drupal_write_record('simplerdf_namespaces', $ns);
  }

  // Rebuild cache
  _simplerdf_namespaces_build(NULL, TRUE);

  if ($saved) {
    drupal_set_message(t('Namespace has been successfully saved.'));
    $form_state['redirect'] = SIMPLERDF_NAMESPACES_ADMIN_PATH;
  }
  else {
    drupal_set_message(t('An unknown error occurred while saving.'), 'error');
  }
}

/**
 * Namespace delete confirm form.
 */
function simplerdf_namespaces_admin_delete_form($form_state, $ns = NULL) {
  // Detect state
  $ns->state = _simplerdf_namespaces_detect_state($ns);

  if ($ns) {
    // Cache namespace
    $form['__ns'] = array(
      '#type' => 'value',
      '#value' => $ns,
    );
  }

  // Construct message keywords
  if ($ns->state == 'overridden') {
    $delete_title = t('Revert');
    $delete = t('revert');
  }
  else {
    $delete_title = t('Delete');
    $delete = t('delete');
  }

  // Prompt user
  return confirm_form($form,
    t('Do you really want to @delete the namespace %ns?', array('@delete' => $delete, '%ns' => $ns->name)),
    SIMPLERDF_NAMESPACES_ADMIN_PATH,
    NULL, $delete_title, t('Cancel'));
}

/**
 * Namespace delete form submit callback.
 * Delete/revert the namespace.
 */
function simplerdf_namespaces_admin_delete_form_submit($form, $form_state) {
  $ns = $form_state['values']['__ns'];
  db_query("DELETE FROM {simplerdf_namespaces} WHERE name = '%s'", $ns->name);

  // Rebuild cache
  _simplerdf_namespaces_build(NULL, TRUE);

  $deleted = $ns->state == 'overridden' ? t('reverted') : t('deleted');
  drupal_set_message(t('The namespace %ns has been @deleted.', array('%ns' => $ns->name, '@deleted' => $deleted)));
  $form_state['redirect'] = SIMPLERDF_NAMESPACES_ADMIN_PATH;
}

/**
 * Detect a namespace object's state.
 */
function _simplerdf_namespaces_detect_state($ns) {
  static $detect_features;
  static $defaults;
  if (!isset($detect_features)) {
    if ($detect_features = module_exists('features')) {
      // Load Features export functions
      module_load_include('inc', 'features', 'features.export');
      $defaults = features_get_default('simplerdf_namespaces');
    }
  }

  $name = $ns->name;
  $state = NULL;
  if ($detect_features) {
    if (is_array($default = $defaults[$name])) {
      // Compare with default namespace
      if ($ns->prefix == $default['prefix'] && $ns->uri == $default['uri']) {
        // Same as default namespace
        $state = 'default';
      }
      else {
        // Modified
        $state = 'overridden';
      }
    }
    else {
      // No matching default namespace
      $state = 'normal';
    }
  }
  else {
    // No default namespaces since Features is absent
    $state = 'normal';
  }

  return $state;
}
