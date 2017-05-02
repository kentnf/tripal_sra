<?php

// dpm($variables); // for debug

// This line is added so that no errors show in the TOC admin page. 
$experiments = $variables['experiment'];
if (sizeof($experiments) > 0) {
  ?>
  <div class="tripal_bioproject-data-block-desc tripal-data-block-desc">Experiments of this BioProject:</div>
  <?php
    
  $headers = array('Experiment Name', 'Description', 'BioSample', 'Organism');
  $rows = array();

  foreach ($experiments as $experiment) {
    $rows[] = array( 
      l($experiment->exp_name, 'experiment/' . $experiment->experiment_id),
      $experiment->description,
      l($experiment->sample_name, 'biosample/' . $experiment->biomaterial_id),
      $experiment->common_name,
    );
  }
  
  $table = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'tripal_experiment-table-properties',
      'class' => 'tripal-data-table table'
    ),
    'sticky' => FALSE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );
  
  print theme_table($table);
}
