<?php
/**
 * @file
 * Implements the bioproject content type of node. 
 */

/**
 * Implements hook_node_access().
 * 
 * @param $node
 * Tee node on which the operation is performed, or, if it does not yet exist, the 
 * type of node to be created.
 *
 * @param $op
 * The operation to be performed. 
 * 
 * @param $account
 * A user object representing the user for whom the operation is to be performed. 
 *
 * @return 
 * If the permission for the specified operation is not set then return FALSE. If the
 * permissions is set then return NULL as this allwos other modules to disable
 * access. The only exception is when the $op == 'create'. We will always
 * return TRUE if the permission is set. 
 * 
 * @ingroup tripal_bioproject
 * 
 */
function chado_bioproject_node_access($node, $op, $account) {
  $node_type = $node;
  if (is_object($node)) {
    $node_type = $node->type;
  }

  if ($node_type == 'chado_bioproject') {
    if ($op == 'create') {
      if (!user_access('create chado_bioproject content', $account)) {
        return NODE_ACCESS_DENY;
      }
      return NODE_ACCESS_ALLOW;
    }
    if ($op == 'update') {
      if (!user_access('edit chado_bioproject content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    if ($op == 'delete') {
      if (!user_access('delete chado_bioproject content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    if ($op == 'view') {
      if (!user_access('access chado_bioproject content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    return NODE_ACCESS_IGNORE;
  }
}

/**
 * Implements hook_form()
 * 
 * @ingroup tripal_bioproejct
 */
function chado_bioproject_form($node, $form_state) {
  $form = array();
 
  // Not sure what this is for. Is this line necessary? 
  $form['#attribute']['enctype'] = 'multipart/form-data';
 
  // === get default value for the form ===
  if (property_exists($node, 'bioproject')) {   
    $bioproject = $node->bioproject;

    // Fill out the form. *** change title in protocl
    $bioproject_name = property_exists($node, 'bioproject_name') ? property_exists($node, 'bioproject_name') : $bioproject->name;
    $description = property_exists($node, 'description') ? property_exists($node, 'description') : $bioproject->description;

    // The conditional here is required because this field is chosen by an optional select.
    //if ($biomaterial->biosourceprovider_id != NULL) {
    //  $biosourceprovider_id = property_exists($node, 'biosourceprovider_id') ? property_exists($node, 'biosourceprovider_id') : $biomaterial->biosourceprovider_id->contact_id;
    //}
    //else {
    //  $biosourceprovider_id = NULL;
    //}
   
    // Set the bioproject id. 
    $form['bioproject_id'] = array(
      '#type' => 'value',
      '#value' => $bioproject->project_id,
    );
    $bioproject_id = $bioproject->project_id;
  }
  else {
    $bioproject_name = property_exists($node, 'bioproject_name') ? property_exists($node, 'bioproject_name') : '';
    $description = property_exists($node, 'description') ? property_exists($node, 'description') : '';
    //$biosourceprovider_id = property_exists($node, 'biosourceprovider_id') ? property_exists($node, 'biosourceprovider_id') : '';
    $bioproject_id = NULL;
  }

  // === construct bioproject form ===
  $form['bioproject_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Bioproject Name (must be unique)'),
    '#required' => TRUE,
    '#default_value' =>  $bioproject_name,
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#rows' => 8,
    '#title' => t('Bioproject Description'),
    '#default_value' => $description,
  );

  // Obtain the list of contacts from the chado contact table. 
  $sql = 'SELECT contact_id, name FROM {contact} ORDER BY name';
  $results = chado_query($sql);
  $contacts = array();
  while ($contact = $results->fetchObject()) {
    $contacts[$contact->contact_id] = $contact->name;
  }

  // ?? why use this form ??
  $form['choose_contact'] = array(
    '#type' => 'value',
    '#value' => $contacts,
  );

  $lt_message = tripal_set_message(t('If the contact of the bioproject is not present in the dropdown list, ')
    . l(t('add a new contact.'), 'node/add/chado-contact') . t(' A bioproject contact is not required.'),
    TRIPAL_INFO, array('return_html' => TRUE)
  );

  $form['biosourceprovider_id'] = array(
    '#title' => t('Project Contact (Principal Investigator)'),
    '#type' => 'select',
    '#empty_value' => '', 
    '#description' => t('The contact is usually Principal Investigator.'),
    //'#default_value' => $biosourceprovider_id,
    '#options' => $contacts,
    '#suffix' => $lt_message,
  );

  // construct bioproject forms using bioproject specific cv
  $bioproject_cv = array(
    'bioproject_data_type',
    'bioproject_sample_scope',
    'bioproject_material',
    'bioproject_capture'
  );
  $node->bioproject_cv = $bioproject_cv; // pass experiment cv term using node

  foreach ($bioproject_cv as $cv_name) {
    // check if the cvterm is selected 
    $cv_value = property_exists($node, $cv_name) ? $node->$cv_name : '';

    $cv = tripal_get_cv(array('name'=>$cv_name));
    $cv = chado_expand_var($cv, 'table', 'cvterm');

    $cv_options = array();
    foreach ($cv->cvterm as $cvterm) {
      $cv_options[$cvterm->cvterm_id] = tripal_bioproject_cvterm_display($cvterm->name, 1);
    }

    $form[$cv_name] = array(
      '#title' => tripal_bioproject_cvterm_display($cv_name, 1),
      '#type' => 'select',
      '#options' => $cv_options,
      '#default_value' => $cv_value,
      '#description' => $cv->definition,
      '#required' => TRUE,
    );
  }

  // PROPERTIES FORM
  //---------------------------------------------
  $prop_cv = tripal_get_default_cv('projectprop', 'type_id');
  $cv_id = $prop_cv ? $prop_cv->cv_id : NULL;
  $details = array(
    'property_table' => 'projectprop',      // the name of the prop table
    'chado_id' => $bioproject_id,           // the value of project_id for this record
    'cv_id' => $cv_id                       // the cv.cv_id of the cv governing projectprop.type_id
  );
  chado_add_node_form_properties($form, $form_state, $details);

  return $form;
}

/**
 * Implements hook_validate().
 * 
 * @ingroup tripal_bioproject
 */
function chado_bioproject_validate($node, $form, &$form_state) {
  // Don't validate if a delete.
  if ($node->op == 'Delete') {
    return;
  }

  // We are syncing if there is an bioproject_id but no node ID. In this case a 
  // validation is not required.  
  if (is_null($node->nid) and property_exists($node, 'bioproject_id') and $node->bioproject_id != 0) {
    return;
  } 

  // Remove white space around field values.
  $node->bioproject_name = trim($node->bioproject_name);
  $node->description = trim($node->description);

 // Validate for an update.  
  if (property_exists($node, 'bioproject_id')) {
    $sql = "
      SELECT * 
      FROM {project} 
      WHERE
        name = :name AND NOT
        project_id = :project_id
    "; 
    $args = array(':name' => $node->bioproject_name, ':project_id' => $node->bioproject_id);
    $result = chado_query($sql, $args)->fetchObject(); 
    if ($result) {
      form_set_error('bioproject_name', t("Update cannot proceed. The bioproject
        '$node->bioproject_name' is already present in the database."));
      tripal_report_error('tripal_sra', TRIPAL_WARNING,
        'Update bioproject: name already exists: %value', 
        array('%value' => "name = $node->bioproject_name"));
    }
  }
  // Validate for an insert.
  else {
    $value = array(
      'name' => $node->bioproject_name,
    );
    $bioproject = chado_select_record('project', array('project_id'), $value);
    if (sizeof($bioproject) > 0) {
      form_set_error('bioproject_name', 'Cannot add the bioproject with this name. 
        The '.$node->bioproject_name.' already exists.');
      tripal_report_error('tripal_sra', TRIPAL_WARNING,
        'Insert bioproject: name already exists: %value',
        array('%value' => "name = $node->bioproject_name")); 
    }
  }     
}  

/**
 * Implements hook_insert().
 * 
 * This function is called on insert of a new node
 * of type 'chado_bioproject' and inserts the necessary information to 
 * public chado_bioproject table. 
 * 
 * @ingroup tripal_bioproject
 */
function chado_bioproject_insert($node) {
  
  // Remove white space around field values.
  $node->bioproject_name = trim($node->bioproject_name);
  $node->description = trim($node->description);

  // Check to see if the node is already associated with a chado record. 
  if (!property_exists ($node, 'bioproject_id')) {
    $values = array(
      'name' => $node->bioproject_name,
      'description' => $node->description,
    );

    //if ($node->biosourceprovider_id) {
    //  $values['biosourceprovider_id'] = $node->biosourceprovider_id;
    //}

    $bioproject = chado_insert_record('project', $values);
    if (!$bioproject) {
      drupal_set_message(t('Unable to add bioproject.'), 'warning');
      tripal_report_error('tripal_sra', TRIPAL_ERROR, 'Insert Bioproject: Unable to create bioproject where values: %values',
      array('%values' => print_r($values, TRUE)));
      return;
    } 
    $bioproject_id = $bioproject['project_id'];

    // * Properties Form *
    $details = array(
      'property_table' => 'projectprop',    // the name of the prop table
      'base_table' => 'project',            // the name of your chado base table
      'foreignkey_name' => 'project_id',    // the name of the key in your base table
      'foreignkey_value' => $bioproject_id  // the value of the biomaterial_id key
    );
    chado_update_node_form_properties($node, $details);

    // insert BioProject Properties to projectprop table
    foreach ($node->bioproject_cv as $cv_name) {
      $values = array(
        'project_id' => $bioproject_id,
        'type_id' => $node->$cv_name,
        'value' => null,
        'rank' => 0,
      );
      $cv_insert = chado_insert_record('projectprop', $values);
      if (!$cv_insert) {
        drupal_set_message(t('Unable to insert BioProject Properties.'), 'warning');
        tripal_report_error('tripal_sra', TRIPAL_ERROR, 'Insert Bioproject Properties: Unable to insert bioproject where values: %values',
        array('%values' => print_r($values, TRUE)));
      }
    }
  }
  else {
    $bioproject_id = $node->bioproject_id;
  } 

  // Check to see if the chado entry has a corresponding chado_bioproject entry.
  // Add if no corresponding entry exists in the chado_bioproject table. 
  $check_org_id = chado_get_id_from_nid('project', $node->nid, 'chado_bioproject');

  if (!$check_org_id) {
    $record = new stdClass();
    $record->nid = $node->nid;
    $record->vid = $node->vid;
    $record->project_id = $bioproject_id;
    drupal_write_record('chado_bioproject', $record);
  }	
} 

/**
 * Implements hook_load().
 * 
 *  When a node is requested by the user this function is called to allo us 
 *  to add auxiliary data to the node object.  
 * 
 * @ingroup tripal_bioproject
 */
function chado_bioproject_load($nodes) {
  foreach ($nodes as $nid => $node) {
    // Find the bioproject nodes and add details. 
    $bioproject_id = chado_get_id_from_nid('project', $nid, 'chado_bioproject');
    // Ignore orphaned nodes.
    if (!$bioproject_id) {
      continue;
    }

    // Build the bioproject variable.
    $values = array('project_id' => $bioproject_id);
    $bioproject = chado_generate_var('project', $values);
    $bioproject = chado_expand_var($bioproject, 'table', 'projectprop'); 

    // Add the description field,
    //$arraydesign = chado_expand_var($arraydesign, 'field', 'arraydesign.description');
    $nodes[$nid]->bioproject = $bioproject;
    $nodes[$nid]->project = $bioproject;

    if (sizeof($bioproject->projectprop) > 0 ) {
      foreach ($bioproject->projectprop as $pp) {
        $cv_name = $pp->type_id->cv_id->name;
        $cv_value = $pp->type_id->cvterm_id;
        $nodes[$nid]->$cv_name = $cv_value;
      }
    }

    // Get title the node title.
    $node->title = chado_get_node_title($node); 
  } 
} 

/**
 * Implements [content_type]_chado_node_default_title_format().
 */ 
function chado_bioproject_chado_node_default_title_format() {
  return '[project.name]';
} 

/**
 * Implements hook_chado_node_default_url_format(). 
 */
function chado_bioproject_chado_node_default_url_format() {
  return '/bioproject/[project.project_id]';
} 

/**
 * Implements hook_node_insert(). 
 * Acts on all content types. 
 * 
 * @ingroup tripal_bioproject
 */ 
function tripal_bioproject_node_insert($node) {
  switch ($node->type) {
    case 'chado_bioproject':
      // Find the a biomaterial and add details.     
      $bioproject_id = chado_get_id_from_nid('project', $node->nid, 'chado_bioproject');
      $values = array('project_id' => $bioproject_id);
      $bioproject = chado_generate_var('project', $values);
      $node->bioproject = $bioproject;
      $node->project = $bioproject;     
      // Set the node title.  
      $node->title = chado_get_node_title($node);

      // Set the URL path.  
      chado_set_node_url($node);
    
      break;
  }
}

/**
 * Implements hook_node_update().
 * Acts on all content types.
 * 
 * @ingroup tripal_bioproject
 */
function tripal_bioproject_node_update($node) {
  switch ($node->type) {
    case 'chado_bioproject':
      $node->title = chado_get_node_title($node);
      chado_set_node_url($node);
      break; 
  }
} 

/**
 * Implements hook_update().
 * 
 * @ingroup tripal_bioproject
 */
function chado_bioproject_update($node) {

  // Remove white space around field values.
  $node->bioproject_name = trim($node->bioproject_name);
  $node->description = trim($node->description);

  $bioproject_id = chado_get_id_from_nid('project', $node->nid, 'chado_bioproject');
 
  if ($node->revision) {
    // Chado does not do revisions but following the example of prior modules
    // keep this here for when/if revisions are addressed in the future. 
  }
  $match = array(
    'project_id' => $bioproject_id,
  );

  //if ($node->biosourceprovider_id == "") {
  //  $biosourceprovider_id = NULL;
  //}
  //else {
  //  $biosourceprovider_id = $node->biosourceprovider_id;
  //}

  $values = array(
    'name' => $node->bioproject_name,
    'description' => $node->description,
  );

  $status = chado_update_record('project', $match, $values);

  if (!$status) {
    drupal_set_message(t('Unable to update bioproject.'), 'warning');
    tripal_report_error('tripal_sra', TRIPAL_WARNING, 'Update bioproject: Unable to update bioproject where values: %values', array('%values' => print_r($values, TRUE)));
  } 

  // * Properties Form *
  $details = array(
    'property_table' => 'projectprop',    // the name of the prop table
    'base_table' => 'project',            // the name of your chado base table
    'foreignkey_name' => 'project_id',    // the name of the key in your base table
    'foreignkey_value' => $bioproject_id  // the value of the bioproject_id key
  );
  chado_update_node_form_properties($node, $details);

  // insert BioProject Properties to projectprop table
  foreach ($node->bioproject_cv as $cv_name) {
    $values = array(
      'project_id' => $bioproject_id,
      'type_id' => $node->$cv_name,
      'value' => null,
      'rank' => 0,
    );
    $cv_insert = chado_insert_record('projectprop', $values);
    if (!$cv_insert) {
      drupal_set_message(t('Unable to insert BioProject Properties.'), 'warning');
      tripal_report_error('tripal_sra', TRIPAL_ERROR, 'Insert Bioproject Properties: Unable to insert bioproject where values: %values',
        array('%values' => print_r($values, TRUE)));
    }
  }
} 

/**
 * Implements hook_delete().
 * 
 * Delete bioproject from both drupal and chado databases. Check dependencies before
 * deleting from chado. 
 * 
 * @ingroup tripal_bioproject
 */
function chado_bioproject_delete($node) {
  $bioproject_id = chado_get_id_from_nid('project', $node->nid, 'chado_bioproject');
  // If there is no bioproject for this node, this is not the correct type or the chado
  // entry is lost. 
  if (!$bioproject_id) {
    return;
  }

  // Remove data from drupal tables including chado_bioproject, node, and node_revisions
  $sql_del = "DELETE FROM {chado_bioproject} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));
  $sql_del = "DELETE FROM {node} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));
  $sql_del = "DELETE FROM {node_revision} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));

  // Check for dependencies before deleting from the chado database.
  // A dependency is the element table. More dependencies may need to be checked
  // in the future if functionality is extended. 
  $check_experiment = chado_query('SELECT experiment_id FROM {experiment} WHERE project_id = :project_id', array(':project_id' => $bioproject_id))->fetchObject();

  if (!$check_experiment) {
    chado_delete_record('projectprop', array('project_id' => $bioproject_id));
    chado_delete_record('project', array('project_id' => $bioproject_id));
    drupal_set_message(t("Warning: the bioproject ".$bioproject_id." has been removed from chado database."), 'status');
  }
  else {
    drupal_set_message(t("Warning: other data depends on this bioproject. The bioproject page was removed from this site but the bioproject record was not removed from Chado."), 'warning'); 
  } 
} 
