<?php

dpm($variables); // for debug
  
// This conditional is added to prevent errors in the experiment TOC admin page.
if (property_exists($variables['node'],'experiment')) {
  $experiment = $variables['node']->experiment;
    
  ?>
  <div class="tripal_experiment-data-block-desc tripal-data-block-desc"></div>
  <?php

  $headers = array();
  $rows = array();
 
  // The experiment name.
  $rows[] = array(
    array(
      'data' => 'Experiment Name',
      'header' => TRUE,
      'width' => '20%',
    ),
    $experiment->name
  );

  // display BioProject
  $rows[] = array(
    array(
      'data' => 'BioProject Name',
      'header' => TRUE,
      'width' => '20%',
    ),
    $experiment->project_id->name
  );

  $rows[] = array(
    array(
      'data' => 'BioProject Description',
      'header' => TRUE,
      'width' => '20%',
    ),
    $experiment->project_id->description
  );

  // display BioSample
  $rows[] = array(
    array(
      'data' => 'BioSample Name',
      'header' => TRUE,
      'width' => '20%',
    ),
    $experiment->biomaterial_id->name
  );

  $rows[] = array(
    array(
      'data' => 'BioSample Description',
      'header' => TRUE,
      'width' => '20%',
    ),
    $experiment->biomaterial_id->description
  );


  // allow site admins to see the experiment ID
  if (user_access('view ids')) {
    // Experiment ID
    $rows[] = array(
      array(
        'data' => 'Experiment ID',
        'header' => TRUE,
        'class' => 'tripal-site-admin-only-table-row',
      ),
      array(
        'data' => $experiment->experiment_id,
        'class' => 'tripal-site-admin-only-table-row',
      ),
    );
  }

  // Generate the table of data provided above. 
  $table = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'tripal_experiment-table-base',
      'class' => 'tripal-experiment-data-table tripal-data-table table',
    ),
    'sticky' => FALSE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );

  // Print the table and the description.
  print theme_table($table); 

  // Print the biomaterial description.
  if ($experiment->description) { ?>
    <div style="text-align: justify"><?php print $experiment->description?></div> <?php
  }

  /**
   * display bioproject property
   */
  $experiment = chado_expand_var($experiment, 'table', 'experimentprop', array('return_array' => 1));
  $properties = $experiment->experimentprop;

  // Check for properties.  
  if (count($properties) > 0) {
    ?>
    <br>
    <div class="tripal_experiment-data-block-desc tripal-data-block-desc">Library Information of this Experiment:</div>
    <?php

    $headers = array('Property Name', 'Value');
    $rows = array();
    $subprop = array();
    foreach ($properties as $property) {
      if ($property->type_id->cv_id->name == 'experiment_property') {
        $subprop[$property->type_id->name] = $property->value;
      } else {
        $cv_name  = tripal_bioproject_cvterm_display($property->type_id->cv_id->name, 1);
        $cv_value = $property->type_id->name;

        if ($cv_value == 'Other') {
          if (!empty($subprop[$property->type_id->cv_id->name])) {
            $cv_value .=  ' (' . $subprop[$property->type_id->cv_id->name] . ')';
            unset($subprop[$property->type_id->cv_id->name]);
          }
        }

        $rows[] = array(
          ucfirst(str_replace('_', ' ', $cv_name)),
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

