<?php
// $Id$
/**
 * @file
 * Administration interfaces.
 */

/** Main administration interface path. */
define('SIMPLERDF_CUSTOM_FIELDS_ADMIN_PATH', 'admin/build/simplerdf/custom_fields');

/**
 * Custom field listing.
 */
function simplerdf_custom_fields_admin_list() {
  $path = drupal_get_path('module', 'simplerdf');
  drupal_add_css($path . '/simplerdf.admin.css');

  $output = '';

  // Custom fields listing table
  $header = array(
    t('Title'),
    t('Machine name'),
    t('Type'),
    t('State'),
    t('Operations'),
  );

  $states = array(
    'normal' => t('Normal'),
    'default' => t('Default'),
    'overridden' => t('Overridden'),
  );

  // Build field rows
  $rows = array();
  foreach (simplerdf_custom_fields_load() as $name => $field) {
    // Build object state
    $field->state = _simplerdf_custom_fields_detect_state($field);

    // Build operations
    $ops = array();

    if ($field->state == 'default') {
      $ops['edit'] = t('Override');
    }
    else {
      $ops['edit'] = t('Edit');
    }
    if ($field->state == 'normal') {
      $ops['delete'] = t('Delete');
    }
    elseif ($field->state == 'overridden') {
      $ops['delete'] = t('Revert');
    }

    foreach ($ops as $op_name => $op) {
      $ops[$op_name] = l($op, SIMPLERDF_CUSTOM_FIELDS_ADMIN_PATH . '/' . $op_name . '/' . $field->name, array('class' => $op_name . '-link'));
    }

    // Build row
    $rows[] = array(
      check_plain($field->title),
      check_plain($field->name),
      check_plain($field->type),
      $states[$field->state],
      array(
        'data' => implode(' | ', $ops),
        'class' => 'simplerdf-custom-fields-operations',
      ),
    );
  }

  // Empty table
  if (empty($rows)) {
    $rows[] = array(
      array(
        'colspan' => count($header),
        'data' => t('No custom fields defined.'),
        'class' => 'empty',
      ),
    );
  }

  // Render
  $output .= theme('table', $header, $rows, array('id' => 'simplerdf-custom-fields-admin-list', 'class' => 'simplerdf-admin-listing-table'));

  $output .= '<div>' . l(t('Add a custom field'), SIMPLERDF_CUSTOM_FIELDS_ADMIN_PATH . '/add') . '</div>';

  return $output;
}

/**
 * Custom field edit form.
 */
function simplerdf_custom_fields_admin_edit_form($form_state, $field = NULL) {
  if (!isset($form_state['values'])) {
    $form_state['values'] = array();
  }

  if ($field) {
    drupal_set_title(t('Edit field %field', array('%field' => $field->name)));

    // Cache value
    $form['__field'] = array(
      '#type' => 'value',
      '#value' => $field,
    );
    $form_state['storage']['last_type'] = $field->type;

    // Edit existing field
    $form_state['values'] += array(
      'type' => $field->type,
      'title' => $field->title,
      'settings' => $field->settings,
    );
  }
  elseif ($form_state['values']['__field']) {
    $field = $form_state['values']['__field'];
  }

  // Stash previous state
  if ($form_state['values']['type']) {
    $form_state['storage']['last_type'] = $form_state['values']['type'];
  }

  // Machine name
  $form['name'] = array(
    '#title' => t('Machine name'),
  );
  if ($field) {
    // Existing item
    $form['name'] += array(
      '#type' => 'item',
      '#value' => $field->name,
    );
  }
  else {
    // New item
    $form['name'] += array(
      '#type' => 'textfield',
      '#maxlength' => 64,
      '#size' => 30,
      '#required' => TRUE,
      '#default_value' => $form_state['values']['name'],
      '#description' => t('Unique code name of this custom field. Must contain only lowercase letters (a-z), numbers (0-9), and underscores (_). Once this value has been set, it cannot be changed.'),
    );
  }

  // Field title
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#description' => t('The user-friendly name of the field.'),
    '#size' => 50,
    '#maxlength' => 255,
    '#default_value' => $form_state['values']['title'],
    '#required' => TRUE,
  );

  // Field type
  $form['field_type'] = array(
    '#type' => 'fieldset',
    '#title' => t('Field type'),
    '#description' => t('Choose the type of field. Either selected option will present different configuration settings for the field.'),
  );
  $form['field_type']['type'] = array(
    '#type' => 'radios',
    '#title' => t('Choose type'),
    '#description' => t('To set the type of the field and configure it using the corresponding form, choose the desired type and press "Apply field type" below.'),
    '#options' => array(
      'basic' => t('Basic'),
      'advanced' => t('Advanced (PHP code)'),
    ),
    '#default_value' => $form_state['values']['type'],
    '#required' => TRUE,
  );
  $form['field_type']['apply_type'] = array(
    '#type' => 'submit',
    '#value' => t('Apply field type'),
    '#validate' => array('_simplerdf_custom_fields_admin_edit_apply'),
  );

  // Show field settings form
  if ($type = $form_state['values']['type']) {
    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => $type == 'basic' ? t('Basic field settings') : t('Advanced field settings'),
      '#tree' => TRUE,
    );

    // Build field settings form
    if ($type == 'basic') {
      $settings_form = _simplerdf_custom_fields_admin_edit_basic_form($form_state);
    }
    else {
      $settings_form = _simplerdf_custom_fields_admin_edit_advanced_form($form_state);
    }
    $form = array_merge_recursive($form, $settings_form);
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save field settings'),
  );

  return $form;
}

/**
 * Field type apply handler.
 */
function _simplerdf_custom_fields_admin_edit_apply($form, &$form_state) {
  // Rebuild form, possibly with new field type option
  $form_state['rebuild'] = TRUE;
}

/**
 * Basic custom field edit form.
 */
function _simplerdf_custom_fields_admin_edit_basic_form(&$form_state) {
  $form['settings']['object_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Applicable object types'),
    '#options' => simplerdf_get_types(),
    '#description' => t('Select the types of objects this field can be mapped to. If none is selected, this field will be effectively unused.'),
  );
  $form['settings']['field_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Field key'),
    '#size' => 30,
    '#max_length' => 64,
    '#default_value' => $form_state['values']['settings']['field_key'],
    '#description' => t('Enter the key of the field to access the object value by. As a PHP note, if the given key is in the variable $key, the expression $object->$key will be used to access the value. For security reasons, the value will be accessed defensively; therefore, make sure this key is valid to obtain any value from the field.'),
  );
  $form['settings']['multiple'] = array(
    '#type' => 'checkbox',
    '#title' => t('Can have multiple values'),
    '#default_value' => $form_state['values']['settings']['multiple'],
    '#description' => t('Select this option if this node field value can optionally have a flat array of values. If the value is not an array, the field will fall back to the default behavior, i.e. treat the field as having a single unstructured value.'),
  );
  return $form;
}

/**
 * Basic custom field edit validator
 */
function _simplerdf_custom_fields_admin_edit_basic_form_validate($form, &$form_state) {
  if ($form_state['values']['type'] != 'basic') {
    return;
  }

  // Test field key is not blank
  if (empty($form_state['values']['settings']['field_key'])) {
    form_set_error('settings][field_key', t('Field key is required.'));
  }
}

/**
 * Advanced custom field edit form.
 */
function _simplerdf_custom_fields_admin_edit_advanced_form(&$form_state) {
  $form['settings']['phpcode'] = array(
    '#type' => 'textarea',
    '#title' => t('PHP code'),
    '#rows' => 10,
    '#default_value' => $form_state['values']['settings']['phpcode'],
  );
  $form['settings']['phphelp'] = array(
    '#type' => 'markup',
    '#value' => theme('item_list', array(
      t('Enter PHP code to map node to one or more values.'),
      t('Do NOT include enclosing !phpopen and !phpclose tags.', array('!phpopen' => '<code>&lt;?php</code>', '!phpclose' => '<code>?&gt;</code>')),
      t('The code should return an unstructured value OR a flat array of values. If nothing is returned, it is assumed that the field does not map to any value.'),
      t('The code is executed once for an entire set of RDF properties for every field.'),
      t('The node object is available as !node.', array('!node' => '<code>$node</code>')),
      t('The !static variable is an statically cached array. The same array is used for all advanced custom field mappings, so namespace your variables accordingly.', array('!static' => '<code>$static</code>')),
      '<strong>' . t('Warning: executing bad code may break your site!') . '</strong>',
    )),
  );
  return $form;
}

/**
 * Advanced custom field edit validator.
 */
function _simplerdf_custom_fields_admin_edit_advanced_form_validate($form, &$form_state) {
  if ($form_state['values']['type'] != 'advanced') {
    return;
  }

  if (empty($form_state['values']['settings']['phpcode'])) {
    form_set_error('settings][phpcode', t('PHP code is required.'));
  }
  else {
    // Normalize PHP newlines
    $phpcode = $form_state['values']['settings']['phpcode'];
    $has_r = strpos($phpcode, "\r") !== FALSE;
    $has_n = strpos($phpcode, "\n") !== FALSE;
    if ($has_r && !$has_n) {
      // Carriage return only (Mac)
      $phpcode = str_replace("\r", "\n", $phpcode);
    }
    elseif ($has_r && $has_n) {
      // Newlines and carriage returns
      $phpcode = str_replace("\r", '', $phpcode);
    }
    $form_state['values']['settings']['phpcode'] = $phpcode;
  }
}

/**
 * Custom field edit form validation callback.
 */
function simplerdf_custom_fields_admin_edit_form_validate(&$form, &$form_state) {
  $name = $form_state['values']['name'];

  // Prepare existing dataset
  $names = array_keys(simplerdf_custom_fields_load());

  // Machine name
  if (!$form_state['values']['__field']) {
    if (preg_match('/[^_a-z0-9]/', $name)) {
      form_set_error('name', t('Machine name contains invalid characters. Please enter only lowercase characters, numbers, and underscores.'));
    }
    elseif (in_array($name, $names)) {
      form_set_error('name', t('Machine name already exists.'));
    }
  }

  // Test whether type has changed
  if ($form_state['values']['type'] && $form_state['storage']['last_type']) {
    if ($form_state['storage']['last_type'] != $form_state['values']['type']) {
      drupal_set_message(t('Please configure settings for the @type field.', array('@type' => $form_state['values']['type'])), 'error');
      // Rebuild form to expose appropriate settings form
      $form_state['rebuild'] = TRUE;
    }
    else {
      // Invoke respective validators
      switch ($form_state['storage']['last_type']) {
        case 'basic':
          _simplerdf_custom_fields_admin_edit_basic_form_validate($form, $form_state);
          break;

        case 'advanced':
          _simplerdf_custom_fields_admin_edit_advanced_form_validate($form, $form_state);
          break;
      }
    }
  }
}

/**
 * Custom field edit form submit callback.
 */
function simplerdf_custom_fields_admin_edit_form_submit($form, &$form_state) {
  $field = new stdClass();
  $field->name = $form_state['values']['__field'] ? $form_state['values']['__field']->name : $form_state['values']['name'];
  $field->title = $form_state['values']['title'];
  $field->type = $form_state['values']['type'];
  $field->settings = (array) $form_state['values']['settings'];

  $saved = FALSE;
  if ($form_state['values']['__field']) {
    $saved = drupal_write_record('simplerdf_custom_fields', $field, 'name');
  }
  else {
    $saved = drupal_write_record('simplerdf_custom_fields', $field);
  }

  // Rebuild cache
  _simplerdf_custom_fields_build(NULL, TRUE);
  simplerdf_clear_cache('fields');
  simplerdf_clear_cache('node');

  if ($saved) {
    drupal_set_message(t('The field settings have been saved.'));
    $form_state['redirect'] = SIMPLERDF_CUSTOM_FIELDS_ADMIN_PATH;
  }
  else {
    drupal_set_message(t('An unknown error occurred while saving.'), 'error');
  }

  // Clear storage
  unset($form_state['storage']['last_type']);
}

/**
 * Field delete confirm form.
 */
function simplerdf_custom_fields_admin_delete_form($form_state, $field = NULL) {
  // Detect state
  $field->state = _simplerdf_custom_fields_detect_state($field);

  if ($field->state == 'default') {
    // Illegal state
    // Go back to the main admin page
    drupal_goto(SIMPLERDF_CUSTOM_FIELDS_ADMIN_PATH);
  }

  if ($field) {
    // Cache field
    $form['__field'] = array(
      '#type' => 'value',
      '#value' => $field,
    );
  }

  // Construct message keywords
  if ($field->state == 'overridden') {
    $delete_title = t('Revert');
    $delete = t('revert');
  }
  else {
    $delete_title = t('Delete');
    $delete = t('delete');
  }

  // Prompt user
  return confirm_form($form,
    t('Do you really want to @delete the field %field?', array('@delete' => $delete, '%field' => $field->name)),
    SIMPLERDF_CUSTOM_FIELDS_ADMIN_PATH,
    NULL, $delete_title, t('Cancel'));
}

/**
 * Field delete form submit callback.
 * Delete/revert the field.
 */
function simplerdf_custom_fields_admin_delete_form_submit($form, &$form_state) {
  $field = $form_state['values']['__field'];
  db_query("DELETE FROM {simplerdf_custom_fields} WHERE name = '%s'", $field->name);

  // Rebuild cache
  _simplerdf_custom_fields_build(NULL, TRUE);
  simplerdf_clear_cache('fields');
  simplerdf_clear_cache('node');

  $deleted = $field->state == 'overridden' ? t('reverted') : t('deleted');
  drupal_set_message(t('The field %field has been @deleted.', array('%field' => $field->name, '@deleted' => $deleted)));
  $form_state['redirect'] = SIMPLERDF_CUSTOM_FIELDS_ADMIN_PATH;
}

/**
 * Detect a custom field object's state.
 */
function _simplerdf_custom_fields_detect_state($field) {
  static $detect_features;
  static $defaults;
  if (!isset($detect_features)) {
    if ($detect_features = module_exists('features')) {
      // Load Features export functions
      module_load_include('inc', 'features', 'features.export');
      $defaults = features_get_default('simplerdf_custom_fields');
    }
  }

  $name = $field->name;
  $state = NULL;
  if ($detect_features) {
    if (is_array($default = $defaults[$name])) {
      // Compare with default namespace
      if ($field->title == $default['title'] && $field->type == $default['type'] && $field->settings == $default['settings']) {
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
