<?php
/**
 * @return the key value array of organism
 */
function tripal_sra_get_organism_select_options() {
  $results = chado_select_record('organism', array('organism_id','common_name'), array());
  foreach ($results as $result) {
    $organism_opts[$result->organism_id] = $result->common_name; 
  }
  return $organism_opts;
}
