<?php

function book_library_preprocess_html(&$vars) {
    $vars['path'] = drupal_get_path('theme', 'book_library');
}

function book_library_preprocess_page(&$vars) {
    global $user;
    $path = drupal_get_path('theme', 'book_library');
    $vars['path'] = $path;
    $vars['current_path'] = current_path();

    if(current_path() == 'user/login') {
        //$form = drupal_get_form('user_login');
        $form = $vars['page']['content']['system_main'];

        $element_arr = array(
            '#description' => '',
            '#attributes' => array(
                //'class' => array('frame_style'),
                'placeholder' => '用户名',
            ),
            '#title' => '',
        );
        $form['name'] = array_merge($form['name'], $element_arr);
        $element_arr = array(
            '#description' => '',
            '#attributes' => array(
                //'class' => array('frame_style'),
                'placeholder' => '密码',
            ),
            '#title' => '',
        );
        $form['pass'] = array_merge($form['pass'], $element_arr);
        $element_arr = array(
            '#description' => '',
            '#attributes' => array(
                //'class' => array('frame_style'),
            ),
            '#title' => '',
        );
        $form['captcha'] = array_merge($form['captcha'], $element_arr);
        $form['captcha']['captcha_widgets']['captcha_image']['#weight'] = 0.001;
        $form['captcha']['captcha_widgets']['captcha_image']['#attributes']['class'] = array('Codes_region');
        $form['captcha']['captcha_widgets']['captcha_response']['#description'] = '';
        $form['captcha']['captcha_widgets']['captcha_response']['#title'] = '';
        $form['captcha']['captcha_widgets']['captcha_response']['#attributes']['placeholder'] = '验证码';

        $element_arr = array(
            '#description' => '',
            '#title' => '保存密码',
            '#attributes' => array(
                'class' => array('inline')
            ),
            '#weight' => 100
            
        );
        $form['persistent_login'] = array_merge($form['persistent_login'], $element_arr);

        $form['actions']['submit']['#attributes']['class'] = array(
            'width-35',
            'pull-right',
            'btn',
            'btn-sm',
            'btn-primary'
        );

        $vars['login_form'] = drupal_render($form);
    } else {
        drupal_add_js('if ("ontouchend" in document) document.write("<script src=\'assets/js/jquery.mobile.custom.min.js\'>" + "<" + "script>");', array('type' => 'inline'));
        drupal_add_js($path . '/assets/js/typeahead-bs2.min.js', array('type' => 'file'));
        drupal_add_js($path . '/assets/js/ace-elements.min.js', array('type' => 'file'));
        drupal_add_js($path . '/assets/js/ace.min.js', array('type' => 'file'));
        drupal_add_js($path . '/assets/layer/layer.js', array('type' => 'file'));
        drupal_add_js($path . '/assets/laydate/laydate.js', array('type' => 'file'));
        drupal_add_js($path . '/js/jquery.nicescroll.js', array('type' => 'file'));

        // 加载菜单
        //$content_menu = menu_load_links('menu-admin-custom-menus');
        //$vars['content_menu'] = $content_menu;

        $types = _node_types_build()->names;
        $vars['content_types'] = $types;

        $structure_menu = menu_load_links('menu-admin-structure-menus');
        $vars['structure_menu'] = $structure_menu;

    }

    $vars['user'] = $user;

    $current_path = current_path();
    $path_arr = explode('/', $current_path);
    $end = max(array_keys($path_arr));
    if(current($path_arr) == 'admin' 
        || (current($path_arr) == 'node' && $path_arr[1] == 'add')
        || (current($path_arr) == 'node' && $path_arr[$end] == 'edit' && is_numeric($path_arr[1]))
        || (current($path_arr) == 'node' && $path_arr[$end] == 'moderation')
        || (current($path_arr) == 'insert')
        || (current($path_arr) == 'content' && $path_arr[$end] == 'content-list')
        || (current($path_arr) == 'content' && $path_arr[$end] == 'content-level')
        ) {
        $admin_path = drupal_get_path('theme', 'adminimal');
        drupal_add_css('/' . $admin_path . '/css/style.css', array('type' => 'external', 'weight' => 100));
        drupal_add_css('/' . $admin_path . '/skins/default/default.css', array('type' => 'external', 'weight' => 101));
    }

    if(end($path_arr) == 'facets-list' || end($path_arr) == 'creator-list') {
        $path = drupal_get_path('theme', 'book_library');
        drupal_add_css('/' . $path . '/css/lightbox.min.css', array('type' => 'external'));
        drupal_add_js($path . '/js/lightbox.min.js', array('type' => 'file'));
    }

    if(isset($_GET['field_shl_donator']) || isset($_GET['field_dc_creator'])) {
        if(end($path_arr) == 'facets-list' || end($path_arr) == 'facets') {
            $start = db_query('SELECT field_dc_date2_value from field_data_field_dc_date2 d LEFT JOIN node ON node.nid = d.entity_id left join field_data_field_shl_donator fsd on d.entity_id = fsd.entity_id WHERE fsd.field_shl_donator_value =:uname ORDER BY d.field_dc_date2_value ASC LIMIT 1;', array(':uname' => $_GET['field_shl_donator']))->fetchField();
            $end = db_query('SELECT field_dc_date2_value from field_data_field_dc_date2 d LEFT JOIN node ON node.nid = d.entity_id left join field_data_field_shl_donator fsd on d.entity_id = fsd.entity_id WHERE fsd.field_shl_donator_value =:uname ORDER BY d.field_dc_date2_value DESC LIMIT 1;', array(':uname' => $_GET['field_shl_donator']))->fetchField();
            $total = db_query('SELECT COUNT(*) FROM field_data_field_shl_donator WHERE field_shl_donator_value = :uname and delta=0;', array(':uname' => $_GET['field_shl_donator']))->fetchField();

          $table = '<table class="table">';
          $date = date_create($start);
          $table .= '<tr><td>首次捐赠时间</td><td>' . date_format($date,'Y-m-d') . '</td></tr>';
          $date = date_create($end);
          $table .= '<tr><td>最近捐赠时间</td><td>' . date_format($date,'Y-m-d') . '</td></tr>';
          $table .= '<tr><td>捐赠总数</td><td>' . $total . '</td></tr>';
          $table .= '</table>';

        }

        if(end($path_arr) == 'creator-list' || end($path_arr) == 'creator') {
            if(isset($_GET['field_dc_creator']))
            {
                $start = db_query('SELECT field_created_value FROM	field_data_field_created d LEFT JOIN node ON node.nid = d.entity_id LEFT JOIN field_data_field_dc_creator fsd ON d.entity_id = fsd.entity_id WHERE	fsd.field_dc_creator_value =:uname ORDER BY d.field_created_value ASC LIMIT 1;', array(':uname' => $_GET['field_dc_creator']))->fetchField();
                $end = db_query('SELECT field_created_value FROM	field_data_field_created d LEFT JOIN node ON node.nid = d.entity_id LEFT JOIN field_data_field_dc_creator fsd ON d.entity_id = fsd.entity_id WHERE	fsd.field_dc_creator_value =:uname ORDER BY d.field_created_value DESC LIMIT 1;', array(':uname' => $_GET['field_dc_creator']))->fetchField();
                $total = db_query('SELECT COUNT(*) FROM field_data_field_dc_creator WHERE field_dc_creator_value = :uname and delta=0 ;', array(':uname' => $_GET['field_dc_creator']))->fetchField();
            }
         $table = '<table class="table">';
         if(valid_date($start))
         {
            $table .= '<tr><td>首次创作时间</td><td>' . date('Y-m-d', $start) . '</td></tr>';
         }
         else
         {
            $table .= '<tr><td>首次创作时间</td><td>' . $start . '</td></tr>';
         }
         if(valid_date($end))
         {
            $table .= '<tr><td>最近创作时间</td><td>' . date('Y-m-d', $end) . '</td></tr>';
         }
         else
         {
            $table .= '<tr><td>最近创作时间</td><td>' . $end . '</td></tr>';
         }
         
          $table .= '<tr><td>创作总数</td><td>' . $total . '</td></tr>';
          $table .= '</table>';
        }

        if(isset($table))
        {
            $vars['juanzeng_date'] = $table;
        }
        
    }
    $vars['messages'] = '';
    if($path_arr[0] == 'node' && (end($path_arr) == 'edit' || $path_arr[1] == 'add')) {
        drupal_get_messages('status');
        drupal_get_messages('warning');
        $vars['messages'] = theme('status_messages');
    }
    
}

function valid_date($date)
{
    //匹配日期格式
    if (preg_match ("/^([0-9]{4})-([0-9]{2})-([0-9]{2})$/", $date, $parts))
    {
        //检测是否为日期,checkdate为月日年
        if(checkdate($parts[2],$parts[3],$parts[1]))
            return true;
        else
            return false;
    }
    else
        return false;
}

/**
 * template_preprocess_views_view
 * 在VBO批量操作表单后增加著录按钮
 */
function book_library_preprocess_views_view(&$vars) {
    if($vars['view']->name == 'custom_facets_pages') {
        $row = $vars['rows'];
    }
}


function book_library_fieldset($variables) {
    $element = $variables['element'];
    element_set_attributes($element, array('id'));
    _form_set_class($element, array('form-wrapper'));

    $out_fiedset = FALSE;
    if(current_path() == 'content/content-list') {
        $out_fiedset = TRUE;
    }
  
    $output = '';
    if($out_fiedset) {
        $output = '<fieldset' . drupal_attributes($element['#attributes']) . '>';
    } else {
        $output = '<div class="form-item">';
    }
    
    if (!empty($element['#title'])) {
      if($out_fiedset) {
          $output .= '<legend>';
      }
      // Always wrap fieldset legends in a SPAN for CSS positioning.
      $output .= '<label class="custom-fieldset-label">' . $element['#title'] . '</label>';
      if($out_fiedset) {
        $output .= '</legend>';
    }
    }

    if($out_fiedset) {
        $output .= '<div class="fieldset-wrapper">';
    }
    
    if (!empty($element['#description'])) {
      $output .= '<div class="fieldset-description">' . $element['#description'] . '</div>';
    }
    
    
    $output .= $element['#children'];
    if (isset($element['#value'])) {
      $output .= $element['#value'];
    }

    if($out_fiedset) {
        $output .= "</div></fieldset>\n";
    } else {
        $output .= '</div>';
    }
    
    return $output;
}
