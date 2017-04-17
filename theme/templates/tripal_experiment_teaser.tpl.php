<?php
$node = $variables['node'];
$experiment = $variables['node']->experiment;
$experiment = chado_expand_var($experiment,'field','experiment.description'); ?>

<div class="tripal_experiment-teaser tripal-teaser">
  <div class="tripal-experiment-teaser-title tripal-teaser-title"><?php
    print l($node->title, "node/$node->nid", array('html' => TRUE));?>
  </div>
  <div class="tripal_experiment-teaser-text tripal-teaser-text"><?php
    print substr($experiment->description, 0, 650);
    if (strlen($experiment->description > 650)) {
      print "... " . l("[more]", "node/$node->nid");
    } ?>
  </div>
</div>

