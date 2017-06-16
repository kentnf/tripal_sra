


### insert below code between line 691 and 692 of tripal_core.chado_nodes.title_and_path.api.inc

file : sites/all/modules/tripal/tripal_core/api/tripal_core.chado_nodes.title_and_path.api.inc

original code
```
691 $node->type = $content_type;
692 $table = preg_replace('/chado_/', '', $content_type);

```
insert code behind line 691, delete line 692
````
          // kentnf: should use base table and linking_table here for universality
          // $table = preg_replace('/chado_/', '', $content_type);
          $module = db_query("SELECT module FROM node_type WHERE type=:type", array(':type'=>$content_type))->fetchField();
          if (!$module) {
            print "Can not get module name of $content_type\n";
            exit;
          }

          $node_info = call_user_func($module . '_node_info');
          $chado_node_api = $node_info[ $content_type ]['chado_node_api'];
          $table = $chado_node_api['base_table'];
          $linking_table = $chado_node_api['linking_table'];
          $id = chado_get_id_from_nid($table, $nid, $linking_table);
          $node->$table = chado_generate_var($table, array($table . "_id" => $id));
          $hook = preg_replace('/chado_/', '', $content_type);
          $node->$hook = $node->$table;
          // kentnf: end

          // Now set the URL for the current node.
          $alias = chado_set_node_url($node);

```

