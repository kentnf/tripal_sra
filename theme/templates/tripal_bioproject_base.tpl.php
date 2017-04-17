<?php

// This conditional is added to prevent errors in the bioproject TOC admin page.
if (property_exists($variables['node'],'bioproject')) {
  $bioproject = $variables['node']->bioproject;
  $bioproject = chado_expand_var($bioproject, 'field', 'project.description');
  //$ontology_terms = $variables['biomaterial_ontology_terms'];

  ?>
  <div class="tripal_biomaterial-data-block-desc tripal-data-block-desc"></div>
  <?php

  //$analysis_name = $variables['analysis_name'];
  //$analysis_nid = $variables['analysis_nid'];

  $headers = array();
  $rows = array();
 
  // The bioproject name.
 
  $rows[] = array(
    array(
      'data' => 'BioProject',
      'header' => TRUE,
      'width' => '20%',
    ),
    '<i>' . $bioproject->name . '</i>'
  );

  // The bioproject contact
  /**
  if($biosample->biosourceprovider_id) {
    $rows[] = array(
      array(
        'data' => 'Biosample Provider',
        'header' => TRUE,
        'width' => '20%',
      ),
      '<i>' . $biosample->biosourceprovider_id->name . '</i>'
    );
  }
  */

  // allow site admins to see the biomaterial ID
  if (user_access('view ids')) {
    // Biomaterial ID
    $rows[] = array(
      array(
        'data' => 'BioPorject ID',
        'header' => TRUE,
        'class' => 'tripal-site-admin-only-table-row',
      ),
      array(
        'data' => $bioproject->project_id,
        'class' => 'tripal-site-admin-only-table-row',
      ),
    );
  }

  // Generate the table of data provided above. 
  $table = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'tripal_bioproject-table-base',
      'class' => 'tripal-bioproject-data-table tripal-data-table table',
    ),
    'sticky' => FALSE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );

  // Print the table and the description.
  print theme_table($table); 

  // Print the bioproject description.
  if ($bioproject->description) { ?>
    <div style="text-align: justify"><?php print $bioproject->description?></div> <?php
  }

  /**
   * display bioproject property
   */
  $bioproject = chado_expand_var($bioproject, 'table', 'projectprop', array('return_array' => 1));
  $properties = $bioproject->projectprop;

  // Check for properties.  
  if (count($properties) > 0) { 
    ?>
    <br>
    <div class="tripal_bioproject-data-block-desc tripal-data-block-desc">Properties this BioBroject:</div>
    <?php

    $headers = array('Property Name', 'Value');
    $rows = array();
    $subprop = array();  
    foreach ($properties as $property) {
      if ($property->type_id->cv_id->name == 'bioproject_property') {
        $subprop[$property->type_id->name] = $property->value;
      } else {
        $cv_name  = tripal_bioproject_cvterm_display($property->type_id->cv_id->name, 1);
        $cv_value = tripal_bioproject_cvterm_display($property->type_id->name, 1);

        if ($cv_value == 'Other') {
          if (!empty($subprop[$property->type_id->cv_id->name])) {
            $cv_value .=  ' (' . $subprop[$property->type_id->cv_id->name] . ')';
            unset($subprop[$property->type_id->cv_id->name]);
          }
        }

        $rows[] = array(
          ucfirst($cv_name),
          $cv_value,
        );
      }
    }

    foreach ($subprop as $name => $value) {
      $rows[] = array(
          ucfirst($name),
          $value,
      );
    }

    $table = array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'tripal_bioproject-table-properties',
        'class' => 'tripal-data-table table'
      ),
      'sticky' => FALSE,
      'caption' => '',
      'colgroups' => array(),
      'empty' => '',
    );
    print theme_table($table);
  }
  // end of display bioproject property
}
?>

