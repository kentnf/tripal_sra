<?php
/**
 * @file
 * Implements the biosample content type of node. 
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
 * @ingroup tripal_biosample
 * 
 */
function chado_biosample_node_access($node, $op, $account) {
  $node_type = $node;
  if (is_object($node)) {
    $node_type = $node->type;
  }

  if ($node_type == 'chado_biosample') {
    if ($op == 'create') {
      if (!user_access('create chado_biosample content', $account)) {
        return NODE_ACCESS_DENY;
      }
      return NODE_ACCESS_ALLOW;
    }
    if ($op == 'update') {
      if (!user_access('edit chado_biosample content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    if ($op == 'delete') {
      if (!user_access('delete chado_biosample content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    if ($op == 'view') {
      if (!user_access('access chado_biosample content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    return NODE_ACCESS_IGNORE;
  }
}

/**
 * Implements hook_form()
 * 
 * When editing or create a new node of type 'chado_biosample' we need
 * a form. This function creates the form that will be used for this. 
 * 
 * @ingroup tripal_biosample
 */
function chado_biosample_form($node, $form_state) {
  $form = array();

  // experimental design for bioproject_property
  tripal_insert_cvterm(array(
    'name' => 'experimental_design',
    'definition' => 'matrix of experimental_design for RNASeq',
    'cv_name' => 'bioproject_property',
    'db_name' => 'tripal_sra',
  ));
 
  // Not sure what this is for. Is this line necessary? 
  $form['#attribute']['enctype'] = 'multipart/form-data';

  // === get default value for the form ===
  if (property_exists($node, 'biosample')) {   
    $biosample = $node->biosample;
    $biosample_name = property_exists($node, 'biosample_name') ? property_exists($node, 'biosample_name') : $biosample->name;
    $description = property_exists($node, 'description') ? property_exists($node, 'description') : $biosample->description;

    // The conditional here is required because this field is chosen by an optional select.
    if ($biosample->biosourceprovider_id != NULL) {
      $biosourceprovider_id = property_exists($node, 'biosourceprovider_id') ? property_exists($node, 'biosourceprovider_id') : $biosample->biosourceprovider_id->contact_id;
    }
    else {
      $biosourceprovider_id = NULL;
    }

    // The conditional here is required because this field is chosen by an optional select.
    if ($biosample->taxon_id != NULL) {
      $taxon_id = property_exists($node, 'taxon_id') ? property_exists($node, 'taxon_id') : $biosample->taxon_id->organism_id;
    }
    else {
      $taxon_id = NULL;
    } 
   
    // Set the biomaterial id. 
    $form['biosample_id'] = array(
      '#type' => 'value',
      '#value' => $biosample->biomaterial_id,
    );
    $biosample_id = $biosample->biomaterial_id;
  }

  else {
    $biosample_name = property_exists($node, 'biosample_name') ? property_exists($node, 'biosample_name') : '';
    $description = property_exists($node, 'description') ? property_exists($node, 'description') : '';
    $biosourceprovider_id = property_exists($node, 'biosourceprovider_id') ? property_exists($node, 'biosourceprovider_id') : '';
    $taxon_id = property_exists($node, 'taxon_id') ? property_exists($node, 'taxon_id') : '';
    $biosample_id = NULL;
  }

  // === construct biosample form ===
  $form['biosample_name'] = array(
    '#type' => 'textfield',
    '#title' => t('BioSample Name (must be unique)'),
    '#required' => TRUE,
    '#default_value' =>  $biosample_name,
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#rows' => 8,
    '#title' => t('BioSample Description'),
    '#default_value' => $description,
  );

  // Obtain the list of contacts from the chado contact table. 
  $sql = 'SELECT contact_id, name FROM {contact} ORDER BY name';
  $results = chado_query($sql);
  $contacts = array();
  while ($contact = $results->fetchObject()) {
    $contacts[$contact->contact_id] = $contact->name;
  }

  $form['choose_contact'] = array(
    '#type' => 'value',
    '#value' => $contacts,
  );

  $lt_message = tripal_set_message(t('If the contact who provided the biosample is not present in the dropdown list, ')
    . l(t('add a new contact.'), 'node/add/chado-contact') . t(' A biosample provider contact is not required.'),
    TRIPAL_INFO, array('return_html' => TRUE)
  );

  $form['biosourceprovider_id'] = array(
    '#title' => t('Biosample Provider'),
    '#type' => 'select',
    '#empty_value' => '', 
    '#description' => t('The contact who provided the biosample.'),
    '#default_value' => $biosourceprovider_id,
    '#options' => $contacts,
    '#suffix' => $lt_message,
  );

  $sql = 'SELECT organism_id, genus, species, common_name FROM {organism} ORDER BY genus';
  $results = chado_query($sql);
  $organisms = array();
  while ($organism = $results->fetchObject()) {
    $organisms[$organism->organism_id] = $organism->genus . ' ' . $organism->species . ' (' . $organism->common_name . ')';
  }

  $lt_message = tripal_set_message(t('If the organism does not appear in this list, ') 
    . l(t('add a new organism.'), 'node/add/chado-organism') . t(' A organism is not required.'),
    TRIPAL_INFO, array('return_html' => TRUE)
  );

  $form['taxon_id'] = array(
    '#title' => t('Organism'),
    '#type' => 'select',
    '#empty_value' => '', 
    '#description' => t('The organism from which the biosample was collected.'),
    '#options' => $organisms,
    '#default_value' => $taxon_id,
    '#suffix' => $lt_message,
  );

  // PROPERTIES FORM
  //---------------------------------------------
  $prop_cv = tripal_get_default_cv('biomaterialprop', 'type_id');
  $cv_id = $prop_cv ? $prop_cv->cv_id : NULL;
  $details = array(
    'property_table' => 'biomaterialprop',  // the name of the prop table
    'chado_id' => $biosample_id,          // the value of feature_id for this record
    'cv_id' => $cv_id                       // the cv.cv_id of the cv governing featureprop.type_id
  );
  chado_add_node_form_properties($form, $form_state, $details);

  // ADDITIONAL DBXREFS FORM
  //---------------------------------------------
  $details = array(
    'linking_table' => 'biomaterial_dbxref',  // the name of the _dbxref table
    'base_foreign_key' => 'biomaterial_id',   // the name of the key in your base chado table
    'base_key_value' => $biosample_id       // the value of feature_id for this record
  );
  chado_add_node_form_dbxrefs($form, $form_state, $details); //*** need to add templates for this add the relationshiops

  // TODO: For some reason adding a relationship to the form breaks AJAX
  // for features (works for other node type)... need to debug

  // RELATIONSHIPS FORM
  //---------------------------------------------
  /**
  $relationship_cv = tripal_get_default_cv('biomaterial_relationship', 'type_id');
  $cv_id = $relationship_cv ? $relationship_cv->cv_id : NULL;
  $details = array(
    'relationship_table' => 'biomaterial_relationship',
    'base_table' => 'biomaterial',
    'base_foreign_key' => 'biomaterial_id',
    'base_key_value' => $biosample_id,
    'base_name_field' => 'name',
    'nodetype' => 'biomaterial',
    'cv_id' => $cv_id
  );
  chado_add_node_form_relationships($form, $form_state, $details);
  */
  return $form;
}

/**
 * Implements hook_validate().
 * 
 * @ingroup tripal_biosample
 */
function chado_biosample_validate($node, $form, &$form_state) {
  // Don't validate if a delete.
  if ($node->op == 'Delete') {
    return;
  }

  // Remove white space around field values.
  // method 1. get biosample info from biosample node form which submit by user
  $node->biosample_name = trim($node->biosample_name);
  $node->description = trim($node->description);

  // method 2. get biosample info from biomaterial table, which is for sync validate
  // the biosample_id is from biomaterial table in sync process 
  if (property_exists($node, 'biomaterial')) {
    $biomaterial = $node->biomaterial;
    if (property_exists($biomaterial, 'biomaterial_id') and empty($node->biosample_name)) {
      $node->biosample_id   = $biomaterial->biomaterial_id;
      $node->biosample_name = $biomaterial->name;
      $node->description    = $biomaterial->description;
    }
  }

  // We are syncing if there is an biomaterial_id but no node ID. In this case a 
  // validation is not required.
  if (is_null($node->nid) and property_exists($node, 'biosample_id') and $node->biosample_id != 0) {
    return;
  } 

  // Validate for an update.  
  if (property_exists($node, 'biosample_id')) {
    $sql = "
      SELECT * 
      FROM {biomaterial} 
      WHERE
        name = :name AND NOT
        biomaterial_id = :biosample_id
    "; 
    $args = array(':name' => $node->biosample_name, ':biosample_id' => $node->biosample_id);
    $result = chado_query($sql, $args)->fetchObject(); 
    if ($result) {
      form_set_error('biosample_name', t("Update cannot proceed. The biosample
        '$node->biosample_name' is already present in the database."));
      tripal_report_error('tripal_sra', TRIPAL_WARNING,
        'Update biosample: name already exists: %value', 
        array('%value' => "name = $node->biosample_name"));
    }
  }
  // Validate for an insert.
  else {
    $value = array(
      'name' => $node->biosample_name,
    );
    $biosample = chado_select_record('biomaterial', array('biomaterial_id'), $value);
    if (sizeof($biosample) > 0) {
      form_set_error('biosample_title', 'Cannot add the biosample with this name. 
        The biosample already exists.');
      tripal_report_error('tripal_sra', TRIPAL_WARNING,
        'Insert biosample: name already exists: %value',
        array('%value' => "name = $node->biosample_name")); 
    }
  }
}  

/**
 * Implements hook_insert().
 * 
 * This function is called on insert of a new node
 * of type 'chado_biosample' and inserts the necessary information to 
 * public chado_biosample table. 
 * 
 * @ingroup tripal_biosample
 */
function chado_biosample_insert($node) {
  
  // Remove white space around field values.
  $node->biosample_name = trim($node->biosample_name);
  $node->description = trim($node->description);

  // Check to see if the node is already associated with a chado record. 
  if (!property_exists ($node, 'biosample_id')) {
    $values = array(
      'name' => $node->biosample_name,
      'description' => $node->description,
    );

    if ($node->biosourceprovider_id) {
      $values['biosourceprovider_id'] = $node->biosourceprovider_id;
    }

    if ($node->taxon_id) { 
      $values['taxon_id'] = $node->taxon_id;
    }

    $biosample = chado_insert_record('biomaterial', $values);
    if (!$biosample) {
      drupal_set_message(t('Unable to add biosample.'), 'warning');
      tripal_report_error('tripal_sra', TRIPAL_ERROR, 'Insert Biosample: Unable to create biosample where values: %values',
      array('%values' => print_r($values, TRUE)));
      return;
    } 
    $biosample_id = $biosample['biomaterial_id'];
    // * Properties Form *
    $details = array(
      'property_table' => 'biomaterialprop',   // the name of the prop table
      'base_table' => 'biomaterial',           // the name of your chado base table
      'foreignkey_name' => 'biomaterial_id',   // the name of the key in your base table
      'foreignkey_value' => $biosample_id      // the value of the biosample_id key
    );
    chado_update_node_form_properties($node, $details);

    // * Additional DBxrefs Form *
    $details = array(
      'linking_table' => 'biomaterial_dbxref',   // the name of your _dbxref table
      'foreignkey_name' => 'biomaterial_id',     // the name of the key in your base table
      'foreignkey_value' => $biosample_id      // the value of the biosample_id key
    );
    chado_update_node_form_dbxrefs($node, $details);
  }
  else {
    $biosample_id = $node->biosample_id;
  }   

  // Check to see if the chado entry has a corresponding chado_biomaterial entry.
  // Add if no corresponding entry exists in the chado_biomaterial table. 
  $check_org_id = chado_get_id_from_nid('biomaterial', $node->nid, 'chado_biosample');
  if (!$check_org_id) {
    $record = new stdClass();
    $record->nid = $node->nid;
    $record->vid = $node->vid;
    $record->biomaterial_id = $biosample_id;
    drupal_write_record('chado_biosample', $record);
  }	
} 

/**
 * Implements hook_load().
 * 
 *  When a node is requested by the user this function is called to allo us 
 *  to add auxiliary data to the node object.  
 * 
 * @ingroup tripal_biosample
 */
function chado_biosample_load($nodes) {
  
  foreach ($nodes as $nid => $node) {
    // Find the biosample nodes and add details. 
    $biosample_id = chado_get_id_from_nid('biomaterial', $nid, 'chado_biosample');
    
    // Ignore orphaned nodes.
    if (!$biosample_id) {
      continue;
    }

    // Build the biomaterial variable.
    $values = array('biomaterial_id' => $biosample_id);
    $biosample = chado_generate_var('biomaterial', $values);
    $biosample->biosample_id = $biosample->biomaterial_id;
 
    // Add the description field,
    $nodes[$nid]->biosample = $biosample;
    $nodes[$nid]->biomaterial = $biosample;

    // Get title the node title.
    $node->title = chado_get_node_title($node); 
  } 
} 

/**
 * Implements [content_type]_chado_node_default_title_format().
 * 
 * Defines a default title format for the Chado Node API to set the titles on 
 * chado_arraydesign nodes based on chado fields.
 */ 
function chado_biosample_chado_node_default_title_format() {
  return '[biomaterial.name]';
} 

/**
 * Implements hook_chado_node_default_url_format(). 
 *
 * Defines a default URL format for arraydesign nodes. 
 */
function chado_biosample_chado_node_default_url_format() {
  return '/biosample/[biomaterial.name]';
} 

/**
 * Implements hook_node_insert(). 
 * Acts on all content types. 
 * 
 * @ingroup tripal_biosample
 */ 
function tripal_biosample_node_insert($node) {
  
  switch ($node->type) {
    case 'chado_biosample':
      // Find the a biomaterial and add details.     
      $biosample_id = chado_get_id_from_nid('biomaterial', $node->nid, 'chado_biosample');
      $values = array('biomaterial_id' => $biosample_id);
      $biosample = chado_generate_var('biomaterial', $values);
      $node->biosample = $biosample;
      $node->biomaterial = $biosample;
      // Set the node title.  
      $node->title = chado_get_node_title($node);
      dpm($node->title, 'node title insert');
      // Set the URL path.  
      chado_set_node_url($node);
      break;
  }
}

/**
 * Implements hook_node_update().
 * Acts on all content types.
 * 
 * @ingroup tripal_biosample
 */
function tripal_biosample_node_update($node) {
  
  switch ($node->type) {
    case 'chado_biosample':
      $node->title = chado_get_node_title($node);
      chado_set_node_url($node);
      break; 
  }
} 

/**
 * Implements hook_update().
 * 
 * @ingroup tripal_biosample
 */
function chado_biosample_update($node) {

  // Remove white space around field values.
  $node->biosample_name = trim($node->biosample_name);
  $node->description = trim($node->description);

  $biosample_id = chado_get_id_from_nid('biomaterial', $node->nid, 'chado_biosample');

  if ($node->revision) {
    // Chado does not do revisions but following the example of prior modules
    // keep this here for when/if revisions are addressed in the future. 
  }
  $match = array(
    'biomaterial_id' => $biosample_id,
  );

  if ($node->biosourceprovider_id == "") {
    $biosourceprovider_id = NULL;
  }
  else {
    $biosourceprovider_id = $node->biosourceprovider_id;
  }

  if ($node->taxon_id == "") {
    $taxon_id = NULL;
  }
  else {
    $taxon_id = $node->taxon_id;
  }

  $values = array(
    'name' => $node->biosample_name,
    'description' => $node->description,
    'biosourceprovider_id' => $biosourceprovider_id,
    'taxon_id' => $taxon_id,
  );

  $status = chado_update_record('biomaterial', $match, $values);

  if (!$status) {
    drupal_set_message(t('Unable to update biosample.'), 'warning');
    tripal_report_error('tripal_sra', TRIPAL_WARNING, 'Update biosample: Unable to update biomsample (biomaterial) where values: %values', array('%values' => print_r($values, TRUE)));
  } 
    
  // * Properties Form *
  $details = array(
    'property_table' => 'biomaterialprop',   // the name of the prop table
    'base_table' => 'biomaterial',           // the name of your chado base table
    'foreignkey_name' => 'biomaterial_id',   // the name of the key in your base table
    'foreignkey_value' => $biosample_id      // the value of the biosample_id key
  );
  chado_update_node_form_properties($node, $details);

  // * Additional DBxrefs Form *
  $details = array(
    'linking_table' => 'biomaterial_dbxref',   // the name of your _dbxref table
    'foreignkey_name' => 'biomaterial_id',     // the name of the key in your base table
    'foreignkey_value' => $biosample_id        // the value of the biosample_id key
  );
  chado_update_node_form_dbxrefs($node, $details);
} 

/**
 * Implements hook_delete().
 * 
 * Delete biosample from both drupal and chado databases. Check dependencies before
 * deleting from chado. 
 * 
 * @ingroup tripal_biosample
 */
function chado_biosample_delete($node) {
  $biosample_id = chado_get_id_from_nid('biomaterial', $node->nid, 'chado_biosample');
 
  // If there is no biomaterial for this node, this is not the correct type or the chado
  // entry is lost. 
  if (!$biosample_id) {
    return;
  }

  // Remove data from drupal tables including chado_biomaterial, node, and node_revisions
  $sql_del = "DELETE FROM {chado_biosample} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));
  $sql_del = "DELETE FROM {node} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));
  $sql_del = "DELETE FROM {node_revision} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));

  // Check for dependencies before deleting from the chado database.
  // A dependency is the element table. More dependencies may need to be checked
  // in the future if functionality is extended. 

  $check_experiment = chado_query('SELECT experiment_id FROM {experiment} WHERE biomaterial_id = :biosample_id', array(':biosample_id' => $biosample_id))->fetchObject();
  $check_biomaterialprop = chado_query('SELECT biomaterialprop_id FROM {biomaterialprop} WHERE biomaterial_id = :biosample_id', array(':biosample_id' => $biosample_id))->fetchObject();

  if (!$check_experiment or !$check_biomaterialprop) {
    chado_delete_record('biomaterial', array('biomaterial_id' => $biosample_id));
    chado_delete_record('biomaterialprop', array('biomaterial_id' => $biosample_id));
  }
  else {
    drupal_set_message(t("Warning: other data depends on this biosample. The biosample page was removed from this site but the biomaterial record was not removed from Chado."), 'warning'); 
  } 
} 

