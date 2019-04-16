<?php
/**
 * 用户首页
 */
function user_front_page() {
    global $user;
    if($user->uid == 0) {
        drupal_goto('user/login');
    }
    return theme('custom_front_page');
}

/**
 * 自定义字段管理页面
 *
 * @param [type] $node_type
 * @return void
 */
function custom_structure_manage($node_type) {
    $path = drupal_get_path('theme', 'adminimal');
    drupal_add_css('/' . $path . '/css/style.css', array('type' => 'external', 'weight' => 100));

    $node_type = str_replace('-', '_', $node_type);
    module_load_include('inc', 'field_ui', 'field_ui.admin');
    $field_form = drupal_get_form('field_ui_field_overview_form', 'node', $node_type);
    return drupal_render($field_form);
}

/**
 * 调用规范档，返回DataTable格式化数据
 *
 * @param String $key
 * @return JSON
 */
function get_api_list() {
    $key = '02cdb77b436d4dc383f1b64ebd86b9dc025622ac';
    if(!isset($_POST['search']) || !isset($_POST['length'])) {
        $response = array(
            'status' => 'error',
            'message' => '参数错误'
        );

        return drupal_json_output($response);
    }

    $keyword = $_POST['search'];
    $length = intval($_POST['length']);
    $length = $length < 0 ? 10 : $length;
    $start = intval($_POST['start']);
    $page = $start > 0 ? $start / $length + 1 : 1;
    $url = "http://data.library.sh.cn/persons/data?freetext=$keyword&key=$key&pageth=$page";

    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl, CURLOPT_HEADER, 0);

    $result = curl_exec($curl);

    curl_close($curl);

    if($result == FALSE) {
        $response = array(
            'status' => 'success',
            'message' => '没有数据'
        );

        return drupal_json_output($response);
    }

    $data = json_decode($result, TRUE);

    if($data['result'] != 0) {
        $response = array(
            'status' => 'success',
            'data' => array(),
        );

        return drupal_json_output($response);
    }

    $response = array();

    foreach($data['data'] as $row) {
        $response['data'][] = array(
            '<a href="javascript:void(0);" class="tooltip-info" data-toggle="tooltip" data-placement="right" title="' . $row['briefBiography'] . '">' . $row['fname'] . '</a>',
            $row['place'],
            isset($row['speciality']) ? $row['speciality'] : '',
            '<a href="javascript:void(0);" data-fname="' . $row['fname'] . '">选择</a>',
            '<a href="javascript:void(0);" data-fname="' . $row['fname'] . '">修改</a>'
        );
    }

    if(!isset($response['data'])) {
        $response['data'] = array();
    }

    $response['draw'] = 0;//$data['pager']['pageth'] - 1;
    $response['recordsTotal'] = $data['pager']['rowCount'];
    $response['recordsFiltered'] = $data['pager']['rowCount'];
    $response['status'] = 'success';

    drupal_json_output($response);
    return;
}

/**
 * 调用地名规范档，返回DataTable格式化数据
 *
 * @param String $key
 * @return JSON
 */
function get_address_api_list() {
    $key = '02cdb77b436d4dc383f1b64ebd86b9dc025622ac';
    if(!isset($_POST['search']) || !isset($_POST['length'])) {
        $response = array(
            'status' => 'error',
            'message' => '参数错误'
        );

        return drupal_json_output($response);
    }

    $keyword = $_POST['search'];
    $length = intval($_POST['length']);
    $length = $length < 0 ? 10 : $length;
    $start = intval($_POST['start']);
    $page = $start > 0 ? $start / $length + 1 : 1;
    $url = "http://data.library.sh.cn/jp/place/$keyword?key=$key&pageth=$page";

    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl, CURLOPT_HEADER, 0);

    $result = curl_exec($curl);

    curl_close($curl);

    if($result == FALSE) {
        $response = array(
            'status' => 'success',
            'message' => '没有数据'
        );

        return drupal_json_output($response);
    }

    $data = json_decode($result, TRUE);

    if($data == null) {
        $response = array(
            'status' => 'success',
            'data' => array(),
        );

        return drupal_json_output($response);
    }

    $province = isset($data['province']) ? $data['province'] : '';
    $province .= '/' . (isset($data['city']) ? $data['city'] : '');
    $country = $data['country'][0]['@language'] == 'chs' ? $data['country'][0]['@value'] : $data['country'][1]['@value'];
    $addr = $data['label'][0]['@language'] == 'chs' ? $data['label'][0]['@value'] : $data['label'][1]['@value'];

    $response = array(
        'data' => array(
            array(
                $country,
                $province,
                $addr,
                '<a href="javascript:void(0);" data-fname="' . $addr . '" data-uri="' . $data['@id'] . '">选择</a>'
            ),
        )
    );

    $response['draw'] = 0;//$data['pager']['pageth'] - 1;
    $response['status'] = 'success';

    drupal_json_output($response);
    return;
}

/**
 * 统计分析页面
 */
function custom_analysis_page($type) {
    $path = drupal_get_path('module', 'custom_admin');
    drupal_add_js($path . '/js/echarts.js', array('type' => 'file'));

    // 获取著录角色用户总数
    $query = db_select('users_roles', 'roles')->fields('roles', array('uid'))->condition('rid', 4);
    $vars['insert_user_count'] = $query->execute()->rowCount();

    // 获取所有节点类型
    $node_types = node_type_get_types();

    $counts = array();
    foreach($node_types as $type) {
        $query = db_select('node', 'node')->fields('node', array('nid', 'type'));
        //$query->rightJoin('node_type', 'node_type', 'node.type = node_type.type');
        $query->condition('type', $type->type);
        $query->addExpression('COUNT(node.nid)', 'node_count');
        $query->where("FROM_UNIXTIME(node.created, '%Y-%m') = :date", array('date' => date('Y-m', REQUEST_TIME)));
        $result = $query->execute()->fetchAll();

        if(count($result) > 0) {
            $counts[$type->type] = $result[0]->node_count;
        }
    }

    $current_month_count_list = array();
    $current_month_count_list = array_map(function($type) use($counts) {
        return array(
            'count' => isset($counts[$type->type]) ? $counts[$type->type] : 0,
            'name' => $type->name
        );
    }, $node_types);
    $vars['current_month_count_list'] = $current_month_count_list;

    // 获取全部数据
    $counts = array();
    foreach($node_types as $type) {
        $query = db_select('node', 'node')->fields('node', array('nid', 'type'));
        //$query->leftJoin('node_type', 'node_type', 'node.type = node_type.type');
        $query->condition('type', $type->type);
        $query->addExpression('COUNT(node.nid)', 'node_count');
        $result = $query->execute()->fetchAll();

        if(count($result) > 0) {
            $counts[$type->type] = $result[0]->node_count;
        }
    }

    $count_list = array_map(function($type) use($counts) {
        return array(
            'count' => isset($counts[$type->type]) ? $counts[$type->type] : 0,
            'name' => $type->name
        );
    }, $node_types);
    $vars['count_list'] = $count_list;

    return theme('custom_analysis_page', array(
        'current_month_count_list' => $current_month_count_list,
        'count_list' => $count_list,
    ));
}

/**
 * 获取内容预览
 */
function ajax_get_node_review($node) {
    $arguments = array(
        'elements' => array(
            '#node' => $node,
            '#view_mode' => 'full'
        )
    );
    echo drupal_render(node_view($node));
}

/**
 * 获取内容预览
 */
function ajax_get_code_review($code) {
    $query = db_select('field_data_field_liushuihao', 'fl')->fields('fl', ['entity_id']);
    $query->condition('field_liushuihao_value', $code);
    $nid = $query->execute()->fetchCol();
    $node = node_load(current($nid));

    echo drupal_render(node_view($node));
}

/**
 * 获取表格内容预览
 */
function ajax_get_node_table_review($node) {
    $node_wrapper = entity_metadata_wrapper('node', $node);
    $types = _node_types_build()->names;

    $rows = array(
        ['索取号', $node_wrapper->field_dct_identifier->value()],
        ['题名', $node->title],
        ['出版时间', date('Y-m-d', $node->created)],
        ['文献类型', $types[$node->type]],
    );

    if(isset($node_wrapper->field_dc_subjectbj)) {
        $rows[] = array('个人名称主题', $node_wrapper->field_dc_subjectbj->value());
    }

    $rows += array(
        ['关键词', $node_wrapper->field_dc_subject->value()],
    );

    if(isset($node_wrapper->field_bf_extent)) {
        $rows[] = array('数量及单位', $node_wrapper->field_bf_extent->value());
    }

    if(isset($node_wrapper->field_bf_dimensions)) {
        $rows[] = array('尺寸', $node_wrapper->field_bf_dimensions->value());
    }

    if(isset($node_wrapper->field_bjzdxs)) {
        $rows[] = array('装订方式', $node_wrapper->field_bjzdxs->value()->name);
    }

    $rows += array(
        ['语种', $node_wrapper->field_dc_language->value()->name],
        ['个人捐赠者', $node_wrapper->field_shl_donator->value()],
        ['捐赠时间', date('Y-m-d', $node_wrapper->field_dc_date2->value())],
        ['一般性附注', $node_wrapper->field_shl_description1->value()]
    );

    if(isset($node_wrapper->field_mi_ji)) {
        $rows[] = array('密级', $node_wrapper->field_mi_ji->value()->name);
    }

    if(isset($node_wrapper->field_qwtp)) {
        $img_url = image_style_url('node_image_review', $node_wrapper->field_qwtp->value()['uri']);
        $rows[] = array('全文图片', '<img src="' . $img_url . '" />');
        //$filename = $node_wrapper->field_sw_qwtp->value()['filename'];
        //$rows[] = array('全文图片', '<a href="' . $img_url . '" target="_blank">' . $filename . '</a>');
    }
    if(isset($node_wrapper->field_sw_qwtp)) {
        $realpath = file_create_url($node_wrapper->field_sw_qwtp->value()['uri']);
        $filename = $node_wrapper->field_sw_qwtp->value()['filename'];
        $rows[] = array('音视频资料', '<a href="' . $realpath . '" target="_blank">' . $filename . '</a>');
    }

    $data = array(
        'header' => array(),
        'rows' => $rows,
        'sticky' => TRUE,
        'empty' => '没有数据',
        'attributes' => array('class' => array('table', 'table-bordered')),
        'caption' => NULL, //count($rows) . 'rows',
        'colgroups' => array()
    );
    $table = theme_table($data);
    echo $table;
}

/**
 * 捐献人
 */
function custom_admin_get_donation_user_list() {
    $page_size = 20;
    $start = isset($_REQUEST['page']) ? $_REQUEST['page'] * $page_size : 0;

    $query_string = 'SELECT field_shl_donator_value, COUNT(bundle) AS count FROM {field_data_field_shl_donator}';
    if(isset($_GET['field_shl_donator'])) {
        $query_string .= ' WHERE field_shl_donator_value LIKE :uname ';
        $query_string .= ' and delta=0 ';
    }
    $query_string .= ' GROUP BY field_shl_donator_value';
    $count_string = "SELECT COUNT(*) AS total FROM ($query_string) AS T";

    $params = array();
    if(isset($_GET['field_shl_donator'])) {
        $params = array(':uname' => '%' . $_GET['field_shl_donator'] . '%');
    }

    $total = db_query($count_string, $params)->fetchField();

    $query = db_query_range($query_string, $start, $page_size, $params);
    $result = $query->fetchAll();

    // 查询捐献批次以及首次和末次时间
    foreach($result as $key => $u) {
        $info_string = "SELECT field_shl_donator_value, COUNT(fdd.bundle) AS count, fdd.field_dc_date2_value AS created FROM {field_data_field_shl_donator} AS fsd";
        //$info_string .= ' LEFT JOIN node ON fsd.entity_id = node.nid';
        $info_string .= ' LEFT JOIN {field_data_field_dc_date2} AS fdd ON fsd.entity_id = fdd.entity_id';
        $info_string .= ' WHERE field_shl_donator_value = :uname and fsd.delta=0 ';
        //$info_string .= " GROUP BY FROM_UNIXTIME(node.created, '%Y-%m-%d')";
        $info_string .= ' GROUP BY field_dc_date2_value';
        $info_result = db_query($info_string, array(':uname' => $u->field_shl_donator_value))->fetchAll();

        $result[$key]->line_count = count($info_result);
        $first = $last = NULL;
        foreach($info_result as $r) {
            if(!empty($r->created)) {
                if(is_null($first)) {
                    $first = date('Y-m-d', strtotime($r->created));
                }
                $last = date('Y-m-d', strtotime($r->created));
            }
        }
        $result[$key]->start = $first;
        $result[$key]->end = $last;
    }

    pager_default_initialize($total, $page_size, $element = 0);
    $pager = theme('pager', array("quantity"=>5));

    $arr = explode('/', current_path());
    $type = end($arr);

    return theme('custom_views_donation_user_list', array('users' => $result, 'type' => $type, 'pager' => $pager));
}

/**
 * 责任者
 */
function custom_admin_get_creator_user_list() {
    $page_size = 20;
    $start = isset($_REQUEST['page']) ? $_REQUEST['page'] * $page_size : 0;

    $query_string = 'SELECT field_dc_creator_value, COUNT(bundle) AS count FROM {field_data_field_dc_creator}';
    if(isset($_GET['field_shl_donator'])) {
        $query_string .= ' WHERE field_dc_creator_value LIKE :uname';
        $query_string .= ' and delta=0 ';
    }
    $query_string .= ' GROUP BY field_dc_creator_value';
    $count_string = "SELECT COUNT(*) FROM ($query_string) AS T";

    $params = array();
    if(isset($_GET['field_shl_donator'])) {
        $params = array(':uname' => '%' . $_GET['field_shl_donator'] . '%');
    }

    $total = db_query($count_string, $params)->fetchField();

    $query = db_query_range($query_string, $start, $page_size, $params);
    $result = $query->fetchAll();

    // 查询捐献批次以及首次和末次时间
    foreach($result as $key => $u) {
        $info_string = "SELECT field_dc_creator_value,fdc.bundle,field_created_value as created,  count(fdc.bundle) as count FROM {field_data_field_dc_creator} AS fdc";
        //$info_string .= ' LEFT JOIN node ON fdc.entity_id = node.nid';
        $info_string .= ' LEFT JOIN {field_data_field_created} AS fcdd ON fdc.entity_id = fcdd.entity_id';
        $info_string .= ' WHERE field_dc_creator_value = :uname and fdc.delta=0 ' ;
        //$info_string .= " GROUP BY FROM_UNIXTIME(node.created, '%Y-%m-%d')";
        $info_string .= ' GROUP BY fdc.bundle,field_created_value';
        $info_result = db_query($info_string, array(':uname' => $u->field_dc_creator_value))->fetchAll();

        $line_arr = array();
        foreach($info_result as $k => $v) {
            $bundle = $v->bundle;
            $type = node_type_load($bundle);

            if(!isset($line_arr[$bundle])) {
                $line_arr[$bundle] = array('name' => $type->name, 'count' => $v->count);
            } else {
                $line_arr[$bundle]['count'] += $v->count;
            }
        }
        $line_count = '';
        foreach($line_arr as $t) {
            $line_count .= $t['name'] . '：' . $t['count'] . '<br/>';
        }
        //$result[$key]->line_count = count($info_result);
        $result[$key]->line_count = $line_count;
        $first = $last = NULL;
        foreach($info_result as $r) {
            if(!empty($r->created)) {
                if(is_null($first)) {
                    if(valid_date($r->created))
                    {
                        $first = date('Y-m-d', $r->created);
                    }
                    else
                    {
                        $first = $r->created;
                    }
                    //$first = date('Y-m-d', strtotime($r->created));
                }
                if(valid_date($r->created))
                {
                    $last = date('Y-m-d', $r->created);
                }
                else
                {
                    $last = $r->created;
                }
                //$last = date('Y-m-d', strtotime($r->created));
            }
        }
        $result[$key]->start = $first;
        $result[$key]->end = $last;
    }

    pager_default_initialize($total, $page_size, $element = 0);
    $pager = theme('pager', array("quantity"=>5));

    $arr = explode('/', current_path());
    $type = end($arr);

    return theme('custom_views_donation_user_list', array('users' => $result, 'type' => $type, 'pager' => $pager));
}

/**
 * 添加人名到地址库
 */
function add_user_to_api() {
    $post_data = $_POST;
    if(count($post_data) != 5) {
        return;
    }

    /*$headers = array(
        "Content-type: application/json;charset='utf-8'",
        "Accept: application/json",
        "Cache-Control: no-cache",
        "Pragma: no-cache",
    );*/

    $url = 'http://data.library.sh.cn/persons/insert4sg';
    foreach($post_data AS $field => $value) {
        $link = current($post_data) == $value ? '?' : '&';
        $url .= "$link$field=" . urlencode($value);
    }
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    //curl_setopt($curl, CURLOPT_HTTPHEADER, $headers);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    //curl_setopt($curl, CURLOPT_POST, 1);
    //curl_setopt($curl, CURLOPT_POSTFIELDS, $post_data);
    $data = curl_exec($curl);

    $result = json_decode($data, TRUE);
    if(isset($result['result'])) {
        $errcode = $result['result'];
        //成功
        if($errcode=="0")
        {
            drupal_json_output(array('status' => 'success'));
        }
        else{
            drupal_json_output(array('status' => 'fail'));
        }
    }

    return;
}

/**
 * 添加地名到地址库
 */
function add_addr_to_api() {
    $post_data = $_POST;
    if(count($post_data) != 5) {
        return;
    }

    $url = 'http://data.library.sh.cn/place/insert';
    foreach($post_data AS $field => $value) {
        $link = current($post_data) == $value ? '?' : '&';
        $url .= "$link$field=" . urlencode($value);
    }
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    //curl_setopt($curl, CURLOPT_HTTPHEADER, $headers);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    //curl_setopt($curl, CURLOPT_POST, 1);
    //curl_setopt($curl, CURLOPT_POSTFIELDS, $post_data);
    $data = curl_exec($curl);

    $result = json_decode($data, TRUE);
    if(isset($result['result'])) {
        drupal_json_output(array('status' => 'success'));
    }
    return;
}

/**
 * 导入数据
 */
function import_history_data() {
    $path = drupal_get_path('module', 'custom_admin');
    drupal_add_js($path . '/js/import_data.js', 'file');
    $html = '<h1>已导入数据：</h1>';
    $html .= '<button id="import_next" data-current="0">导入下一批</button>';
    return $html;
}

/**
 * 插入纸质资料数据
 */
function import_history_zhizhiziliao_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'zhizhiziliao';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;

    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=strtotime($data['last_update_time']);
    }

    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
    $node_wrapper->field_dc_creator->set(array($data['PersonalAndrole']['personal']));
    // 责任方式
    if(isset($data['PersonalAndrole']['personalRole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['PersonalAndrole']['personalRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role->set(array($tt));
    }

    // 团体责任者
    if(isset($data['CorporateAndrole'])) {
        $val = is_array($data['CorporateAndrole']['corporate']) ? $data['CorporateAndrole']['corporate'] : array($data['CorporateAndrole']['corporate']);
        $node_wrapper->field_dc_creator4->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['CorporateAndrole']['corporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role1->set(array($tt));
    }
    // 出版社
    if(isset($data['Publication'])) {
        $val = is_array($data['Publication']['placeOfPublication']) ? $data['Publication']['placeOfPublication'] : array($data['Publication']['placeOfPublication']);
        $node_wrapper->field__di_useapi->set($val);
        $val = is_array($data['Publication']['dc_publisher']) ? $data['Publication']['dc_publisher'] : array($data['Publication']['dc_publisher']);
        $node_wrapper->field_dc_publisher->set($val);

        if(isset($data['Publication']['dcterms_issued'])) {
            $str = str_replace('[?]', '', $data['Publication']['dcterms_issued']);
            $str = str_replace('[不详]', '', $data['Publication']['dcterms_issued']);
            $str = str_replace('不详', '', $data['Publication']['dcterms_issued']);
            $str = str_replace('日', '.', $data['Publication']['dcterms_issued']);
            $str = str_replace('年', '.', $data['Publication']['dcterms_issued']);
            $str = str_replace('月', '.', $data['Publication']['dcterms_issued']);
            $str = str_replace('.', '-', $data['Publication']['dcterms_issued']);
            $ta = explode('-', $str);
            $ta = array_map(function($item) {
                return trim($item);
            }, $ta);
            if(!isset($ta[1])) $ta[1] = '01';
            if(!isset($ta[2])) $ta[2] = '01';
            $str = implode('-', $ta);
            $str .= ' 00:00:00';
            if(strtotime($str)) {
                $node_wrapper->field_cb_dc_date->set(array(array('value' => $str, 'value2' => $str)));
            }
        }


    }
    //文献类型
    if(isset($data['dc_type'])) {
        $val = is_array($data['dc_type']) ? $data['dc_type'] : array($data['dc_type']);
        $node_wrapper->field_bjwxlx->set($val);
    }
    if(isset($data['nameOfPerson'])) {
        $val = is_array($data['nameOfPerson']) ? $data['nameOfPerson'] : array($data['nameOfPerson']);
        $node_wrapper->field_dc_subjectbj->set($val);
    }

    if(isset($data['SubjectOfcorporate'])) {
        //团体名称
        $val = is_array($data['SubjectOfcorporate']['nameOfCorporation']) ? $data['SubjectOfcorporate']['nameOfCorporation'] : array($data['SubjectOfcorporate']['nameOfCorporation']);
        $node_wrapper->field_dc_tt_subject->set($val);
        //会议届次
        if(isset($data['SubjectOfcorporate']['numberOfConference'])) {
            $val = is_array($data['SubjectOfcorporate']['numberOfConference']) ? $data['SubjectOfcorporate']['numberOfConference'] : array($data['SubjectOfcorporate']['numberOfConference']);
            $node_wrapper->field_bjhyjc->set($val);
        }
        //会议地点
        if(isset($data['SubjectOfcorporate']['placeOfConference'])) {
            $val = is_array($data['SubjectOfcorporate']['placeOfConference']) ? $data['SubjectOfcorporate']['placeOfConference'] : array($data['SubjectOfcorporate']['placeOfConference']);
            $node_wrapper->field_shl_place->set($val);
        }

        // 会议时间
        if(isset($data['SubjectOfcorporate']['dateOfConference'])) {
            $str = str_replace('[?]', '', $data['SubjectOfcorporate']['dateOfConference']);
            $str = str_replace('[不详]', '', $data['SubjectOfcorporate']['dateOfConference']);
            $str = str_replace('不详', '',  $data['SubjectOfcorporate']['dateOfConference']);
            $str = str_replace('日', '.',  $data['SubjectOfcorporate']['dateOfConference']);
            $str = str_replace('年', '.',  $data['SubjectOfcorporate']['dateOfConference']);
            $str = str_replace('月', '.',  $data['SubjectOfcorporate']['dateOfConference']);
            $str = str_replace('.', '-', $data['SubjectOfcorporate']['dateOfConference']);
            $ta = explode('-', $str);
            $ta = array_map(function($item) {
                return trim($item);
            }, $ta);
            if(!isset($ta[1])) $ta[1] = '01';
            if(!isset($ta[2])) $ta[2] = '01';
            $str = implode('-', $ta);
            $str .= ' 00:00:00';
            if(strtotime($str)) {
                $node_wrapper->field_shl_temporalvalues->set(array(array('value' => $str, 'value2' => $str)));
            }
        }
    }
    //历史事件主题
    if(isset($data['subjectOfEvents'])) {
        $val = is_array($data['subjectOfEvents']) ? $data['subjectOfEvents'] : array($data['subjectOfEvents']);
        $node_wrapper->field_lssj_dc_subject->set($val);
    }
    //地名主题
    if(isset($data['place'])) {
        $val = is_array($data['place']) ? $data['place'] : array($data['place']);
        $node_wrapper->field_dmzt_dc_subject->set($val);
    }
    //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
    //数量及单位
    if(isset($data['amount'])) {
        $val = is_array($data['amount']) ? $data['amount'] : array($data['amount']);
        $node_wrapper->field_bf_extent->set($val);
    }
    //尺寸
    if(isset($data['size'])) {
        $val = is_array($data['size']) ? $data['size'] : array($data['size']);
        $node_wrapper->field_bf_dimensions->set($val);
    }

    // 装订方式
    if(isset($data['descriptionOfBinding'])) {
        $tax = taxonomy_vocabulary_machine_name_load('taxonomy_binding_type');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['descriptionOfBinding']) ? $data['descriptionOfBinding'] : array($data['descriptionOfBinding']);
        foreach($tree as $term) {
            foreach ($val as $item) {
             if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
            $node_wrapper->field_bjzdxs->set($tt);
    }

    if(isset($data['dcterms_abstract'])) {
        $val = is_array($data['dcterms_abstract']) ? $data['dcterms_abstract'] : array($data['dcterms_abstract']);
        $node_wrapper->field_dct_abstract->set($val);
    }

    // 获取方式
    if(isset($data['cdwa_availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['cdwa_availability']) ? $data['cdwa_availability'] : array($data['cdwa_availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_shl_obtainedby->set($tt);
    }

    // 语种
    if(!isset($data['dc_language']) || $data['dc_language'] == '中文') $data['dc_language'] = '汉语';
    if(isset($data['dc_language'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['dc_language']) ? $data['dc_language'] : array($data['dc_language']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }

    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }

    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }

    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }

    if(isset($data['dc_rights'])) {
        $val = is_array($data['dc_rights']) ? $data['dc_rights'] : array($data['dc_rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }

    if(isset($data['dc_description'])) {
        $val = is_array($data['dc_description']) ? $data['dc_description'] : array($data['dc_description']);
        $node_wrapper->field_shl_description1->set($val);
    }
    if(isset($data['notesPertainingToPhysicalDescription'])) {
        $val = is_array($data['notesPertainingToPhysicalDescription']) ? $data['notesPertainingToPhysicalDescription'] : array($data['notesPertainingToPhysicalDescription']);
        $node_wrapper->field_shl_description2->set($val);
    }
    if(isset($data['RelatedResource']))
    {
        $val = is_array($data['RelatedResource']['dcterms_reference']) ? $data['RelatedResource']['dcterms_reference'] : array($data['RelatedResource']['dcterms_reference']);
        $node_wrapper->field_reference->set($val);
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }
    if(isset($data['dcterms_reference'])) {

    }
    if(isset($data['callNoOfRelatedResource'])) {

    }
    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }

    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}
/**
 * 插入实物数据
 */
function import_history_shiwu_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'shiwu';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;
    //$node->uid = $user->uid;
    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=$data['last_update_time'];
    }

    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
    $node_wrapper->field_dc_creator->set(array($data['PersonalAndRole']['personal']));
    // 责任方式
    if(isset($data['PersonalAndRole']['personalRole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = null;
        foreach($tree as $term) {
            if($term->name == $data['PersonalAndRole']['personalRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role->set(array($tt));
    }

    // 团体责任者
    if(isset($data['CorporateAndRole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $val = is_array($data['CorporateAndRole']['corporate']) ? $data['CorporateAndRole']['corporate'] : array($data['CorporateAndRole']['corporate']);
        $node_wrapper->field_dc_creator4->set($val);

        $tt = null;
        foreach($tree as $term) {
            if($term->name == $data['CorporateAndRole']['corporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role1->set(array($tt));
    }
    // 个人其他责任者
    if(isset($data['ContributorPersonalAndRole'])) {
        $val = is_array($data['ContributorPersonalAndRole']['contributorPersonal']) ? $data['ContributorPersonalAndRole']['contributorPersonal'] : array($data['ContributorPersonalAndRole']['contributorPersonal']);
        $node_wrapper->field_bf_agent->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['ContributorPersonalAndRole']['contributorPersonalRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role2->set(array($tt));
    }
     // 团体其他责任者
     if(isset($data['ContributorCorporateAndRole'])) {
        $val = is_array($data['ContributorCorporateAndRole']['contributorCorporate']) ? $data['ContributorCorporateAndRole']['contributorCorporate'] : array($data['ContributorCorporateAndRole']['contributorCorporate']);
        $node_wrapper->field_bf_agent1->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = null;
        foreach($tree as $term) {
            if($term->name == $data['ContributorCorporateAndRole']['contributorCorporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role3->set(array($tt));
    }
    //创作地
    if(isset($data['locationOfCreation'])) {
        $val = is_array($data['locationOfCreation']) ? $data['locationOfCreation'] : array($data['locationOfCreation']);
        $node_wrapper->field__di_useapi->set($val);
    }
    // 创作时间
    // if(isset($data['created'])) {
    //     $str = str_replace('[?]', '', $data['created']);
    //     $str = str_replace('[不详]', '', $data['created']);
    //     $str = str_replace('不详', '', $data['created']);
    //     $str = str_replace('日', '.', $data['created']);
    //     $str = str_replace('年', '.', $data['created']);
    //     $str = str_replace('月', '.', $data['created']);
    //     $str = str_replace('.', '-', $data['created']);
    //     $ta = explode('-', $str);
    //     $ta = array_map(function($item) {
    //         return trim($item);
    //     }, $ta);
    //     if(!isset($ta[1])) $ta[1] = '01';
    //     if(!isset($ta[2])) $ta[2] = '01';
    //     $str = implode('-', $ta);
    //     $str .= ' 00:00:00';
    //     if(strtotime($str)) {
    //         $node_wrapper->field_dct_created->set(array(array('value' => $str, 'value2' => $str)));
    //     }
    // }
    //创作时间
    if(isset($data['created'])) {
        $val = is_array($data['created']) ? $data['created'] : array($data['created']);
        $node_wrapper->field_created->set($val);
    }
    //文献类型
    if(isset($data['type'])) {
        $tax = taxonomy_vocabulary_machine_name_load('wenxianleixing');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['type']) ? $data['type'] : array($data['type']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_dc_type1->set($tt);
    }
    //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
    //尺寸
    if(isset($data['size'])) {
        $val = is_array($data['size']) ? $data['size'] : array($data['size']);
        $node_wrapper->field_bf_dimensions->set($val);
    }
    //材质
    if(isset($data['materials'])) {
        $tax = taxonomy_vocabulary_machine_name_load('caizhi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['materials']) ? $data['materials'] : array($data['materials']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_shl_material->set($tt);
    }

    //色泽
    if(isset($data['color'])) {
        $tax = taxonomy_vocabulary_machine_name_load('seze');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['color']) ? $data['color'] : array($data['color']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_shl_color1->set($tt);
    }
    // 获取方式
    if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_obtainedby->set($tt);
        }

    }
    //语种
    if(!isset($data['language']) || $data['language'] == '中文') $data['language'] = '汉语';
    if(isset($data['language'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);

        $tt = array();
        $val = is_array($data['language']) ? $data['language'] : array($data['language']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }
    //个人捐赠者
    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
   //权限
    if(isset($data['Rights'])) {
        $val = is_array($data['Rights']) ? $data['Rights'] : array($data['Rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!empty($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }
    //一般性附注
    if(isset($data['dc_description'])) {
        $val = is_array($data['dc_description']) ? $data['dc_description'] : array($data['dc_description']);
        $node_wrapper->field_shl_description1->set($val);
    }
    //载体形态附注
    if(isset($data['notesPertainingToPhysicalDescription'])) {
        $val = is_array($data['notesPertainingToPhysicalDescription']) ? $data['notesPertainingToPhysicalDescription'] : array($data['notesPertainingToPhysicalDescription']);
        $node_wrapper->field_shl_description2->set($val);
    }
    if(isset($data['RelatedResource']))
    {
        //相关资源名
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        //相关资源索取号
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }

    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }
    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}
/**
 * 插入证件数据
 */
function import_history_zhengjian1_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'zhengjian';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;

    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=$data['last_update_time'];
    }

    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
    $node_wrapper->field_dc_creator->set(array($data['CorporateAndrole']['corporate']));
    // 责任方式
    if(isset($data['CorporateAndrole']['corporateRole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['CorporateAndrole']['corporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role->set(array($tt));
    }
    // 个人其他责任者
    if(isset($data['ContributorPersonalAndRole'])) {
        $val = is_array($data['ContributorPersonalAndRole']['contributorPersonal']) ? $data['ContributorPersonalAndRole']['contributorPersonal'] : array($data['ContributorPersonalAndRole']['contributorPersonal']);
        $node_wrapper->field_bf_agent->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['ContributorPersonalAndRole']['contributorPersonalRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role2->set(array($tt));
    }
     // 团体其他责任者
     if(isset($data['ContributorCorporateAndRole'])) {
        $val = is_array($data['ContributorCorporateAndRole']['contributorCorporate']) ? $data['ContributorCorporateAndRole']['contributorCorporate'] : array($data['ContributorCorporateAndRole']['contributorCorporate']);
        $node_wrapper->field_bf_agent1->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['ContributorCorporateAndRole']['contributorCorporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role3->set(array($tt));
    }

    //文献类型
    if(isset($data['type'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_zhengj');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['type']) ? $data['type'] : array($data['type']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_dc_type2->set($tt);
    }
    //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
     // 颁发时间
    //  if(isset($data['created'])) {
    //     $str = str_replace('[?]', '', $data['created']);
    //     $str = str_replace('[不详]', '', $data['created']);
    //     $str = str_replace('不详', '', $data['created']);
    //     $str = str_replace('日', '.', $data['created']);
    //     $str = str_replace('年', '.', $data['created']);
    //     $str = str_replace('月', '.', $data['created']);
    //     $str = str_replace('.', '-', $data['created']);
    //     $ta = explode('-', $str);
    //     $ta = array_map(function($item) {
    //         return trim($item);
    //     }, $ta);
    //     if(!isset($ta[1])) $ta[1] = '01';
    //     if(!isset($ta[2])) $ta[2] = '01';
    //     $str = implode('-', $ta);
    //     $str .= ' 00:00:00';
    //     if(strtotime($str)) {
    //         $node_wrapper->field_dct_created->set(array(array('value' => $str, 'value2' => $str)));
    //     }
    // }
    //创作时间
    if(isset($data['created'])) {
        $val = is_array($data['created']) ? $data['created'] : array($data['created']);
        $node_wrapper->field_created->set($val);
    }
    //尺寸
    if(isset($data['size'])) {
        $val = is_array($data['size']) ? $data['size'] : array($data['size']);
        $node_wrapper->field_bf_dimensions->set($val);
    }
    //内容提要
    if(isset($data['abstracts_xml'])) {
        $val = is_array($data['abstracts_xml']) ? $data['abstracts_xml'] : array($data['abstracts_xml']);
        $node_wrapper->field_dct_abstract->set($val);
    }
    // 获取方式
    if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_obtainedby->set($tt);
        }

    }
    //语种
    if(!isset($data['language']) || $data['language'] == '中文') $data['language'] = '汉语';
    if(isset($data['language'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);

        $tt = array();
        $val = is_array($data['language']) ? $data['language'] : array($data['language']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }
    //个人捐赠者
    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
   //权限
    if(isset($data['Rights'])) {
        $val = is_array($data['Rights']) ? $data['Rights'] : array($data['Rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!empty($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }
    //一般性附注
    if(isset($data['dc_description'])) {
        $val = is_array($data['dc_description']) ? $data['dc_description'] : array($data['dc_description']);
        $node_wrapper->field_shl_description1->set($val);
    }
    //载体形态附注
    if(isset($data['notesPertainingToPhysicalDescription'])) {
        $val = is_array($data['notesPertainingToPhysicalDescription']) ? $data['notesPertainingToPhysicalDescription'] : array($data['notesPertainingToPhysicalDescription']);
        $node_wrapper->field_shl_description2->set($val);
    }

    if(isset($data['RelatedResource']))
    {
        //相关资源名
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        //相关资源索取号
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }
    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }
    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}

/**
 * 插入笔记数据
 */
function import_history_biji_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'biji';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;

    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=$data['last_update_time'];
    }
    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
    $node_wrapper->field_dc_creator->set(array($data['CreatorAndrole']['personal']));
    // 责任方式
    if(isset($data['CreatorAndrole']['personalrole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['CreatorAndrole']['personalrole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!empty($tt))
            $node_wrapper->field_bf_role->set(array($tt));
    }
     //文献类型
    if(isset($data['type'])) {
        $val = is_array($data['type']) ? $data['type'] : array($data['type']);
        $node_wrapper->field_bjwxlx->set($val);
    }
     //个人名称主题
     if(isset($data['nameOfPerson'])) {
        $val = is_array($data['nameOfPerson']) ? $data['nameOfPerson'] : array($data['nameOfPerson']);
        $node_wrapper->field_dc_subjectbj->set($val);
    }
    //团体名称主题
    if(isset($data['SubjectOfcorporate'])) {
        //团体名称
        $val = is_array($data['SubjectOfcorporate']['nameOfCorporation']) ? $data['SubjectOfcorporate']['nameOfCorporation'] : array($data['SubjectOfcorporate']['nameOfCorporation']);
        $node_wrapper->field_dc_tt_subject->set($val);
        //会议届次
        if(isset($data['SubjectOfcorporate']['numberOfConference'])) {
            $val = is_array($data['SubjectOfcorporate']['numberOfConference']) ? $data['SubjectOfcorporate']['numberOfConference'] : array($data['SubjectOfcorporate']['numberOfConference']);
            $node_wrapper->field_bjhyjc->set($val);
        }
        //会议地点
        if(isset($data['SubjectOfcorporate']['placeOfConference'])) {
            $val = is_array($data['SubjectOfcorporate']['placeOfConference']) ? $data['SubjectOfcorporate']['placeOfConference'] : array($data['SubjectOfcorporate']['placeOfConference']);
            $node_wrapper->field_shl_place->set($val);
        }

        // 会议时间
        if(isset($data['SubjectOfcorporate']['dateOfConference'])) {
            $str = str_replace('.', '-', $data['SubjectOfcorporate']['dateOfConference']);
            $ta = explode('-', $str);
            $ta = array_map(function($item) {
                return trim($item);
            }, $ta);
            if(!isset($ta[1])) $ta[1] = '01';
            if(!isset($ta[2])) $ta[2] = '01';
            $str = implode('-', $ta);
            $str .= ' 00:00:00';
            if(strtotime($str)) {
                $node_wrapper->field_shl_temporalvalues->set(array(array('value' => $str, 'value2' => $str)));
            }
        }
    }

     //历史事件主题
     if(isset($data['subjectOfEvents'])) {
        $val = is_array($data['subjectOfEvents']) ? $data['subjectOfEvents'] : array($data['subjectOfEvents']);
        $node_wrapper->field_lssj_dc_subject->set($val);
    }
    //地名主题
    if(isset($data['place'])) {
        $val = is_array($data['place']) ? $data['place'] : array($data['place']);
        $node_wrapper->field_dmzt_dc_subject->set($val);
    }

    //创作地
    if(isset($data['locationOfCreation'])) {
        $val = is_array($data['locationOfCreation']) ? $data['locationOfCreation'] : array($data['locationOfCreation']);
        $node_wrapper->field__di_useapi->set($val);
    }
    // 创作时间
    // if(isset($data['created'])) {
    //     $str = $data['created'];
    //     $list = explode('-', $str);
    //     $count = count($list);
    //     if($count>0)
    //     {
    //         $val1=$list[0];
    //         $val2 =$list[1];

    //         $str1 = str_replace('[?]', '', $val1);
    //         $str1 = str_replace('[不详]', '', $val1);
    //         $str1 = str_replace('不详', '', $val1);
    //         $str1 = str_replace('日', '.', $val1);
    //         $str1 = str_replace('年', '.', $val1);
    //         $str1 = str_replace('月', '.', $val1);
    //         $str1 = str_replace('.', '-', $val1);
    //         $ta1 = explode('-', $str1);
    //         $ta1 = array_map(function($item) {
    //             return trim($item);
    //         }, $ta1);
    //         if(!isset($ta1[1])) $ta1[1] = '01';
    //         if(!isset($ta1[2])) $ta1[2] = '01';
    //         $str1 = implode('-', $ta1);
    //         $str1 .= ' 00:00:00';

    //         $str2 = str_replace('[?]', '', $val2);
    //         $str2 = str_replace('[不详]', '', $val2);
    //         $str2 = str_replace('不详', '', $val2);
    //         $str2 = str_replace('日', '.', $val2);
    //         $str2 = str_replace('年', '.', $val2);
    //         $str2 = str_replace('月', '.', $val2);
    //         $str2 = str_replace('.', '-', $val2);
    //         $ta2 = explode('-', $str2);
    //         $ta2 = array_map(function($item) {
    //             return trim($item);
    //         }, $ta2);
    //         if(!isset($ta2[1])) $ta2[1] = '01';
    //         if(!isset($ta2[2])) $ta2[2] = '01';
    //         $str2 = implode('-', $ta2);
    //         $str2 .= ' 00:00:00';

    //         if(strtotime($str1)&&strtotime($str2)) {
    //             $node_wrapper->field_dct_created->set(array(array('value' => $str1, 'value2' => $str2)));
    //         }
    //     }
    //     else
    //     {
    //         $str = str_replace('[?]', '', $data['created']);
    //         $str = str_replace('[不详]', '', $data['created']);
    //         $str = str_replace('不详', '', $data['created']);
    //         $str = str_replace('日', '.', $data['created']);
    //         $str = str_replace('年', '.', $data['created']);
    //         $str = str_replace('月', '.', $data['created']);
    //         $str = str_replace('.', '-', $data['created']);
    //         $ta = explode('-', $str);
    //         $ta = array_map(function($item) {
    //             return trim($item);
    //         }, $ta);
    //         if(!isset($ta[1])) $ta[1] = '01';
    //         if(!isset($ta[2])) $ta[2] = '01';
    //         $str = implode('-', $ta);
    //         $str .= ' 00:00:00';
    //         if(strtotime($str)) {
    //             $node_wrapper->field_dct_created->set(array(array('value' => $str, 'value2' => $str)));
    //         }
    //     }
    // }
     //创作时间
     if(isset($data['created'])) {
        $val = is_array($data['created']) ? $data['created'] : array($data['created']);
        $node_wrapper->field_created->set($val);
    }

    //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
     //数量及单位
     if(isset($data['amount'])) {
        $val = is_array($data['amount']) ? $data['amount'] : array($data['amount']);
        $node_wrapper->field_bf_extent->set($val);
    }
    //尺寸
    if(isset($data['size'])) {
        $val = is_array($data['size']) ? $data['size'] : array($data['size']);
        $node_wrapper->field_bf_dimensions->set($val);
    }

    //装订方式
    if(isset($data['descriptionOfBinding'])) {
        $tax = taxonomy_vocabulary_machine_name_load('taxonomy_binding_type');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['descriptionOfBinding']) ? $data['descriptionOfBinding'] : array($data['descriptionOfBinding']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_bjzdxs->set($tt);
    }
     //书写工具
     if(isset($data['ImplementAndcolor']['implement'])) {
        $tax = taxonomy_vocabulary_machine_name_load('shuxiegongju');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['ImplementAndcolor']['implement']) ? $data['ImplementAndcolor']['implement'] : array($data['ImplementAndcolor']['implement']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_shl_writingtool->set($tt);
    }
    //色泽
    if(isset($data['ImplementAndcolor']['color'])) {
        $tax = taxonomy_vocabulary_machine_name_load('seze');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['ImplementAndcolor']['color']) ? $data['Implementandcolor']['color'] : array($data['ImplementAndcolor']['color']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_shl_writingcolor->set($tt);
    }
    // 获取方式
    if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_obtainedby->set($tt);
        }

    }
    //语种
    if(!isset($data['language']) || $data['language'] == '中文') $data['language'] = '汉语';
    if(isset($data['language'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);

        $tt = array();
        $val = is_array($data['language']) ? $data['language'] : array($data['language']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }
    //个人捐赠者
    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
   //权限
    if(isset($data['Rights'])) {
        $val = is_array($data['Rights']) ? $data['Rights'] : array($data['Rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!empty($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }
    //一般性附注
    if(isset($data['dc_description'])) {
        $val = is_array($data['dc_description']) ? $data['dc_description'] : array($data['dc_description']);
        $node_wrapper->field_shl_description1->set($val);
    }
     //内容提要
     if(isset($data['dcterms_abstract'])) {
        $val = is_array($data['dcterms_abstract']) ? $data['dcterms_abstract'] : array($data['dcterms_abstract']);
        $node_wrapper->field_dct_abstract->set($val);
    }
    //载体形态附注
    if(isset($data['notesPertainingToPhysicalDescription'])) {
        $val = is_array($data['notesPertainingToPhysicalDescription']) ? $data['notesPertainingToPhysicalDescription'] : array($data['notesPertainingToPhysicalDescription']);
        $node_wrapper->field_shl_description2->set($val);
    }
    if(isset($data['RelatedResource']))
    {
        //相关资源名
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        //相关资源索取号
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }
    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }
    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}
/**
 * 插入证书数据
 */
function import_history_zhengshu_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'zhengshu';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;
    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=$data['last_update_time'];
    }
    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
    $node_wrapper->field_dc_creator->set(array($data['CorporateAndRole']['corporate']));
    // 责任方式
    if(isset($data['CorporateAndRole']['corporateRole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['CorporateAndRole']['corporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role->set(array($tt));
    }

    // 个人其他责任者
    if(isset($data['ContributorPersonalAndrole'])) {
        $val = is_array($data['ContributorPersonalAndrole']['contributorPersonal']) ? $data['ContributorPersonalAndrole']['contributorPersonal'] : array($data['ContributorPersonalAndrole']['contributorPersonal']);
        $node_wrapper->field_bf_agent->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['ContributorPersonalAndrole']['contributorPersonalRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role2->set(array($tt));
    }
     // 团体其他责任者
     if(isset($data['ContributorCorporateAndrole'])) {
        $val = is_array($data['ContributorCorporateAndrole']['contributorCorporate']) ? $data['ContributorCorporateAndrole']['contributorCorporate'] : array($data['ContributorCorporateAndrole']['contributorCorporate']);
        $node_wrapper->field_bf_agent1->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['ContributorCorporateAndrole']['contributorCorporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role3->set(array($tt));
    }

    //文献类型
    if(isset($data['type'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_zhengshu');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['type']) ? $data['type'] : array($data['type']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_zswxlx->set($tt);
    }
    //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
     // 颁发时间
    //  if(isset($data['created'])) {
    //     $str = str_replace('[?]', '', $data['created']);
    //     $str = str_replace('[不详]', '', $data['created']);
    //     $str = str_replace('不详', '', $data['created']);
    //     $str = str_replace('日', '.', $data['created']);
    //     $str = str_replace('年', '.', $data['created']);
    //     $str = str_replace('月', '.', $data['created']);
    //     $str = str_replace('.', '-', $data['created']);
    //     $ta = explode('-', $str);
    //     $ta = array_map(function($item) {
    //         return trim($item);
    //     }, $ta);
    //     if(!isset($ta[1])) $ta[1] = '01';
    //     if(!isset($ta[2])) $ta[2] = '01';
    //     $str = implode('-', $ta);
    //     $str .= ' 00:00:00';
    //     if(strtotime($str)) {
    //         $node_wrapper->field_dct_created->set(array(array('value' => $str, 'value2' => $str)));
    //     }
    // }
    //创作时间
    if(isset($data['created'])) {
        $val = is_array($data['created']) ? $data['created'] : array($data['created']);
        $node_wrapper->field_created->set($val);
    }
    //尺寸
    if(isset($data['size'])) {
        $val = is_array($data['size']) ? $data['size'] : array($data['size']);
        $node_wrapper->field_bf_dimensions->set($val);
    }
    //内容提要
    if(isset($data['abstracts_xml'])) {
        $val = is_array($data['abstracts_xml']) ? $data['abstracts_xml'] : array($data['abstracts_xml']);
        $node_wrapper->field_dct_abstract->set($val);
    }
    // 获取方式
    if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_obtainedby->set($tt);
        }

    }
    //语种
    if(!isset($data['language']) || $data['language'] == '中文') $data['language'] = '汉语';
    if(isset($data['language'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);

        $tt = array();
        $val = is_array($data['language']) ? $data['language'] : array($data['language']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }
    //个人捐赠者
    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
   //权限
    if(isset($data['Rights'])) {
        $val = is_array($data['Rights']) ? $data['Rights'] : array($data['Rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!empty($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }
    //一般性附注
    if(isset($data['dc_description'])) {
        $val = is_array($data['dc_description']) ? $data['dc_description'] : array($data['dc_description']);
        $node_wrapper->field_shl_description1->set($val);
    }
    //载体形态附注
    if(isset($data['notesPertainingToPhysicalDescription'])) {
        $val = is_array($data['notesPertainingToPhysicalDescription']) ? $data['notesPertainingToPhysicalDescription'] : array($data['notesPertainingToPhysicalDescription']);
        $node_wrapper->field_shl_description2->set($val);
    }
    if(isset($data['RelatedResource']))
    {
        //相关资源名
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        //相关资源索取号
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }
    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }
    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}
/**
 * 插入日记数据
 */
function import_history_riji_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'riji';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;
    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=strtotime($data['last_update_time']);
    }
    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
    //其他题名
    if(isset($data['other'])) {
        $val = is_array($data['other']) ? $data['other'] : array($data['other']);
        $node_wrapper->field_shl_varianttitle->set($val);
    }
     //作者
     if(isset($data['personal'])) {
        $val = is_array($data['personal']) ? $data['personal'] : array($data['personal']);
        $node_wrapper->field_dc_creator->set($val);
    }
      //文献类型
    if(isset($data['type'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_riji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['type']) ? $data['type'] : array($data['type']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_ri_dc_type->set($tt);
    }
     //个人名称主题
     if(isset($data['nameOfPerson'])) {
        $val = is_array($data['nameOfPerson']) ? $data['nameOfPerson'] : array($data['nameOfPerson']);
        $node_wrapper->field_dc_subjectbj->set($val);
    }
    //团体名称主题
    if(isset($data['Corporatesubject'])) {
        //团体名称
        $val = is_array($data['Corporatesubject']['nameOfCorporation']) ? $data['Corporatesubject']['nameOfCorporation'] : array($data['Corporatesubject']['nameOfCorporation']);
        $node_wrapper->field_dc_tt_subject->set($val);
        //会议届次
        if(isset($data['Corporatesubject']['numberOfConference'])) {
            $val = is_array($data['Corporatesubject']['numberOfConference']) ? $data['Corporatesubject']['numberOfConference'] : array($data['Corporatesubject']['numberOfConference']);
            $node_wrapper->field_bjhyjc->set($val);
        }
        //会议地点
        if(isset($data['Corporatesubject']['placeOfConference'])) {
            $val = is_array($data['Corporatesubject']['placeOfConference']) ? $data['Corporatesubject']['placeOfConference'] : array($data['Corporatesubject']['placeOfConference']);
            $node_wrapper->field_shl_place->set($val);
        }

        // 会议时间
        if(isset($data['Corporatesubject']['dateOfConference'])) {
            $str = str_replace('.', '-', $data['Corporatesubject']['dateOfConference']);
            $ta = explode('-', $str);
            $ta = array_map(function($item) {
                return trim($item);
            }, $ta);
            if(!isset($ta[1])) $ta[1] = '01';
            if(!isset($ta[2])) $ta[2] = '01';
            $str = implode('-', $ta);
            $str .= ' 00:00:00';
            if(strtotime($str)) {
                $node_wrapper->field_shl_temporalvalues->set(array(array('value' => $str, 'value2' => $str)));
            }
        }
    }

     //历史事件主题
     if(isset($data['subjectOfEvents'])) {
        $val = is_array($data['subjectOfEvents']) ? $data['subjectOfEvents'] : array($data['subjectOfEvents']);
        $node_wrapper->field_lssj_dc_subject->set($val);
    }
    //地名主题
    if(isset($data['place'])) {
        $val = is_array($data['place']) ? $data['place'] : array($data['place']);
        $node_wrapper->field_dmzt_dc_subject->set($val);
    }

    //创作地
    if(isset($data['locationOfCreation'])) {
        $val = is_array($data['locationOfCreation']) ? $data['locationOfCreation'] : array($data['locationOfCreation']);
        $node_wrapper->field__di_useapi->set($val);
    }

     //创作时间
     if(isset($data['created'])) {
        $val = is_array($data['created']) ? $data['created'] : array($data['created']);
        $node_wrapper->field_created->set($val);
    }

    //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
     //数量及单位
     if(isset($data['amount'])) {
        $val = is_array($data['amount']) ? $data['amount'] : array($data['amount']);
        $node_wrapper->field_bf_extent->set($val);
    }
    //尺寸
    if(isset($data['SIZE'])) {
        $val = is_array($data['SIZE']) ? $data['SIZE'] : array($data['SIZE']);
        $node_wrapper->field_bf_dimensions->set($val);
    }

    //装订方式
    if(isset($data['descriptionOfBinding'])) {
        $tax = taxonomy_vocabulary_machine_name_load('taxonomy_binding_type');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['descriptionOfBinding']) ? $data['descriptionOfBinding'] : array($data['descriptionOfBinding']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_bjzdxs->set($tt);
    }
     //书写工具
     if(isset($data['Implementandcolor']['implement'])) {
        $tax = taxonomy_vocabulary_machine_name_load('shuxiegongju');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['Implementandcolor']['implement']) ? $data['Implementandcolor']['implement'] : array($data['Implementandcolor']['implement']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_shl_writingtool->set($tt);
    }
    //色泽
    if(isset($data['Implementandcolor']['color'])) {
        $tax = taxonomy_vocabulary_machine_name_load('seze');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['Implementandcolor']['color']) ? $data['Implementandcolor']['color'] : array($data['Implementandcolor']['color']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_shl_writingcolor->set($tt);
    }
    // 获取方式
    if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_obtainedby->set($tt);
        }

    }
    //语种
    if(!isset($data['LANGUAGE']) || $data['LANGUAGE'] == '中文') $data['LANGUAGE'] = '汉语';
    if(isset($data['LANGUAGE'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);

        $tt = array();
        $val = is_array($data['LANGUAGE']) ? $data['LANGUAGE'] : array($data['LANGUAGE']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }
    //个人捐赠者
    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
   //权限
    if(isset($data['Rights'])) {
        $val = is_array($data['Rights']) ? $data['Rights'] : array($data['Rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!empty($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }
    //一般性附注
    if(isset($data['dc_description'])) {
        $val = is_array($data['dc_description']) ? $data['dc_description'] : array($data['dc_description']);
        $node_wrapper->field_shl_description1->set($val);
    }
     //内容提要
     if(isset($data['abstracts_xml'])) {
        $val = is_array($data['abstracts_xml']) ? $data['abstracts_xml'] : array($data['abstracts_xml']);
        $node_wrapper->field_dct_abstract->set($val);
    }
    //载体形态附注
    if(isset($data['notesPertainingToPhysicalDescription'])) {
        $val = is_array($data['notesPertainingToPhysicalDescription']) ? $data['notesPertainingToPhysicalDescription'] : array($data['notesPertainingToPhysicalDescription']);
        $node_wrapper->field_shl_description2->set($val);
    }
    if(isset($data['RelatedResource']))
    {
        //相关资源名
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        //相关资源索取号
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }
    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }
    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}
/**
 * 插入音像资料数据
 */
function import_history_yinxiangziliao_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'yinxiangziliao';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;
    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=strtotime($data['last_update_time']);
    }
    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
    $node_wrapper->field_shl_isbn->set($data['iSBN']);

    $node_wrapper->field_dc_creator->set(array($data['PersonalAndRole']['personal']));
    // 责任方式
    if(isset($data['PersonalAndRole']['personalRole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = null;
        foreach($tree as $term) {
            if($term->name == $data['PersonalAndRole']['personalRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role->set(array($tt));
    }

    // 团体责任者
    if(isset($data['CorporateAndRole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $val = is_array($data['CorporateAndRole']['corporate']) ? $data['CorporateAndRole']['corporate'] : array($data['CorporateAndRole']['corporate']);
        $node_wrapper->field_dc_creator4->set($val);

        $tt = null;
        foreach($tree as $term) {
            if($term->name == $data['CorporateAndRole']['corporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role1->set(array($tt));
    }
    // 个人其他责任者
    if(isset($data['ContributorPersonalAndRole'])) {
        $val = is_array($data['ContributorPersonalAndRole']['contributorpersonal']) ? $data['ContributorPersonalAndRole']['contributorpersonal'] : array($data['ContributorPersonalAndRole']['contributorpersonal']);
        $node_wrapper->field_bf_agent->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['ContributorPersonalAndRole']['contributorpersonalRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role2->set(array($tt));
    }
     // 团体其他责任者
     if(isset($data['ContributorCorporateAndRole'])) {
        $val = is_array($data['ContributorCorporateAndRole']['contributorCorporate']) ? $data['ContributorCorporateAndRole']['contributorCorporate'] : array($data['ContributorCorporateAndRole']['contributorCorporate']);
        $node_wrapper->field_bf_agent1->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = null;
        foreach($tree as $term) {
            if($term->name == $data['ContributorCorporateAndRole']['contributorCorporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role3->set(array($tt));
    }
     // 获取方式
     if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_obtainedby->set($tt);
        }

    }
    //数量及单位
    if(isset($data['amount'])) {
        $val = is_array($data['amount']) ? $data['amount'] : array($data['amount']);
        $node_wrapper->field_bf_extent->set($val);
    }
    //片长
    if(isset($data['size'])) {
        $val = is_array($data['size']) ? $data['size'] : array($data['size']);
        $node_wrapper->field_bf_dimensions->set($val);
    }
    //录制地
    if(isset($data['placeofCreation'])) {
        $val = is_array($data['placeofCreation']) ? $data['placeofCreation'] : array($data['placeofCreation']);
        $node_wrapper->field__di_useapi->set($val);
    }

    //录制时间
    if(isset($data['dcterms_created'])) {
        $val = is_array($data['dcterms_created']) ? $data['dcterms_created'] : array($data['dcterms_created']);
        $node_wrapper->field_created->set($val);
    }
    //开始时码
    if(isset($data['timeOfStart'])) {
        $val = is_array($data['timeOfStart']) ? $data['timeOfStart'] : array($data['timeOfStart']);
        $node_wrapper->field_timeofstart->set($val);
    }
    //结束时码
    if(isset($data['timeOfEnd'])) {
        $val = is_array($data['timeOfEnd']) ? $data['timeOfEnd'] : array($data['timeOfEnd']);
        $node_wrapper->field_timeofend->set($val);
    }
    //文献类型
    if(isset($data['dc_type'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yxzlwxlx');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['dc_type']) ? $data['dc_type'] : array($data['dc_type']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_wxlxyxzl->set($tt);
    }
    //个人名称
    if(isset($data['nameOfPerson'])) {
        $val = is_array($data['nameOfPerson']) ? $data['nameOfPerson'] : array($data['nameOfPerson']);
        $node_wrapper->field_dc_subjectbj->set($val);
    }
    //位置
    if(isset($data['location'])) {
        $val = is_array($data['location']) ? $data['location'] : array($data['location']);
        $node_wrapper->field_shl_personappperance->set($val);
    }
    //出现时码
     if(isset($data['timeOfApperance'])) {
        $val = is_array($data['timeOfApperance']) ? $data['timeOfApperance'] : array($data['timeOfApperance']);
        $node_wrapper->field_shl_timeofapperance->set($val);
    }
    //团体名称主题
    if(isset($data['nameOfCorporation'])) {
        $val = is_array($data['nameOfCorporation']) ? $data['nameOfCorporation'] : array($data['nameOfCorporation']);
        $node_wrapper->field_dc_tt_subject->set($val);
    }
     //历史事件主题
     if(isset($data['subjectOfEvents'])) {
        $val = is_array($data['subjectOfEvents']) ? $data['subjectOfEvents'] : array($data['subjectOfEvents']);
        $node_wrapper->field_lssj_dc_subject->set($val);
    }
    //地名主题
    if(isset($data['place'])) {
        $val = is_array($data['place']) ? $data['place'] : array($data['place']);
        $node_wrapper->field_dmzt_dc_subject->set($val);
    }
    //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
     //关键片断描述
     if(isset($data['descriptionOfKeysegment'])) {
        $val = is_array($data['descriptionOfKeysegment']) ? $data['v'] : array($data['descriptionOfKeysegment']);
        $node_wrapper->field_shl_keysegment->set($val);
    }
    //关键片断开始码
    if(isset($data['startOfKeysegment'])) {
        $val = is_array($data['startOfKeysegment']) ? $data['startOfKeysegment'] : array($data['startOfKeysegment']);
        $node_wrapper->field_shl_startofkeysegment->set($val);
    }
    //关键片断结束码
    if(isset($data['endOfKeysegment'])) {
        $val = is_array($data['endOfKeysegment']) ? $data['endOfKeysegment'] : array($data['endOfKeysegment']);
        $node_wrapper->field_shl_endofkeysegment->set($val);
    }
     //声音描述
     if(isset($data['descriptionOfVoice'])) {
        $val = is_array($data['descriptionOfVoice']) ? $data['descriptionOfVoice'] : array($data['descriptionOfVoice']);
        $node_wrapper->field_shl_voice->set($val);
    }

    //颜色
    if(isset($data['cdwa_color'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_yxzl');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['cdwa_color']) ? $data['cdwa_color'] : array($data['cdwa_color']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_yxzl_shl_color->set($tt);
    }
    //语种
    if(!isset($data['dc_language']) || $data['dc_language'] == '中文') $data['dc_language'] = '汉语';
    if(isset($data['dc_language'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);

        $tt = array();
        $val = is_array($data['dc_language']) ? $data['dc_language'] : array($data['dc_language']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }
    //个人捐赠者
    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
   //权限
    if(isset($data['Rights'])) {
        $val = is_array($data['Rights']) ? $data['Rights'] : array($data['Rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!empty($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }
    //一般性附注
    if(isset($data['description'])) {
        $val = is_array($data['description']) ? $data['description'] : array($data['description']);
        $node_wrapper->field_shl_description1->set($val);
    }
    //内容提要
    if(isset($data['dcterms_abstract'])) {
        $val = is_array($data['dcterms_abstract']) ? $data['dcterms_abstract'] : array($data['dcterms_abstract']);
        $node_wrapper->field_dct_abstract->set($val);
    }
    // 载体形式
    if(isset($data['descriptionOfBinding'])) {
        $tax = taxonomy_vocabulary_machine_name_load('ztxs');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['descriptionOfBinding']) ? $data['descriptionOfBinding'] : array($data['descriptionOfBinding']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_shl_carrier->set($tt);
    }
    if(isset($data['RelatedResource']))
    {
        //相关资源名
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        //相关资源索取号
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }

    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }
    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}
/**
 * 插入照片数据
 */
function import_history_zhaopian_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'zhaopian';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;

    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=strtotime($data['last_update_time']);
    }
    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
    $node_wrapper->field_shl_serialno->set($data['controlNo']);

    // 获取方式
    if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
            $node_wrapper->field_shl_obtainedby->set($tt);
        }

    }

     //拍摄者
     if(isset($data['creator'])) {
        $val = is_array($data['creator']) ? $data['creator'] : array($data['creator']);
        $node_wrapper->field_dc_creator->set($val);
    }
    //拍摄地
    if(isset($data['locationOfCreation'])) {
        $val = is_array($data['locationOfCreation']) ? $data['locationOfCreation'] : array($data['locationOfCreation']);
        $node_wrapper->field__di_useapi->set($val);
    }

     //创作时间
     if(isset($data['created'])) {
        $val = is_array($data['created']) ? $data['created'] : array($data['created']);
        $node_wrapper->field_created->set($val);
    }
    //拍摄质量
    if(isset($data['quality'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_zhiliang');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['quality']) ? $data['quality'] : array($data['quality']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_pszl->set($tt);
    }
    //尺寸
    if(isset($data['size'])) {
        $val = is_array($data['size']) ? $data['size'] : array($data['size']);
        $node_wrapper->field_bf_dimensions->set($val);
    }
    //底片类别
    if(isset($data['typeOfFilm'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_leib');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['typeOfFilm']) ? $data['typeOfFilm'] : array($data['typeOfFilm']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_dplb->set($tt);
    }
     //底片规格
     if(isset($data['sizeOfFilm'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_guige');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['sizeOfFilm']) ? $data['sizeOfFilm'] : array($data['sizeOfFilm']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_shl_sizeoffilm->set($tt);
    }
    //颜色
    if(isset($data['color'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_yanse');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['color']) ? $data['color'] : array($data['color']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_zp_shl_color->set($tt);
    }
    //翻拍自
    if(isset($data['source'])) {
        $val = is_array($data['source']) ? $data['source'] : array($data['source']);
        $node_wrapper->field_shl_remarkfrom->set($val);
    }
    //文献类型
    if(isset($data['type'])) {
        $tax = taxonomy_vocabulary_machine_name_load('zpwxlx');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['type']) ? $data['type'] : array($data['type']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_field_zp_dc_type->set($tt);
    }
     //人物
     if(isset($data['PersonLocation']['person'])) {
        $val = is_array($data['PersonLocation']['person']) ? $data['PersonLocation']['person'] : array($data['PersonLocation']['person']);
        $node_wrapper->field_rj_dc_creator->set($val);
    }
     //位置
     if(isset($data['PersonLocation']['location'])) {
        $val = is_array($data['PersonLocation']['location']) ? $data['PersonLocation']['location'] : array($data['PersonLocation']['location']);
        $node_wrapper->field_location->set($val);
    }
    //团体名称主题
    if(isset($data['Corporate'])) {
        //团体名称
        $val = is_array($data['Corporate']['nameOfCorporation']) ? $data['Corporate']['nameOfCorporation'] : array($data['Corporate']['nameOfCorporation']);
        $node_wrapper->field_dc_tt_subject->set($val);
        //会议届次
        if(isset($data['Corporate']['numberOfConference'])) {
            $val = is_array($data['Corporate']['numberOfConference']) ? $data['Corporate']['numberOfConference'] : array($data['Corporate']['numberOfConference']);
            $node_wrapper->field_bjhyjc->set($val);
        }
        //会议地点
        if(isset($data['Corporate']['placeOfConference'])) {
            $val = is_array($data['Corporate']['placeOfConference']) ? $data['Corporate']['placeOfConference'] : array($data['Corporate']['placeOfConference']);
            $node_wrapper->field_shl_place->set($val);
        }

        // 会议时间
        if(isset($data['Corporate']['dateOfConference'])) {
            $str = str_replace('.', '-', $data['Corporate']['dateOfConference']);
            $ta = explode('-', $str);
            $ta = array_map(function($item) {
                return trim($item);
            }, $ta);
            if(!isset($ta[1])) $ta[1] = '01';
            if(!isset($ta[2])) $ta[2] = '01';
            $str = implode('-', $ta);
            $str .= ' 00:00:00';
            if(strtotime($str)) {
                $node_wrapper->field_shl_temporalvalues->set(array(array('value' => $str, 'value2' => $str)));
            }
        }
    }

    //地名
    if(isset($data['place'])) {
        $val = is_array($data['place']) ? $data['place'] : array($data['place']);
        $node_wrapper->field_dmzt_dc_subject->set($val);
    }

    //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
    //时间
    if(isset($data['time'])) {
        $val = is_array($data['time']) ? $data['time'] : array($data['time']);
        $node_wrapper->field_zptime->set($val);
    }

     //事件
     if(isset($data['events'])) {
        $val = is_array($data['events']) ? $data['events'] : array($data['events']);
        $node_wrapper->field_event->set($val);
    }
    //一般性附注
    if(isset($data['dc_description'])) {
        $val = is_array($data['dc_description']) ? $data['dc_description'] : array($data['dc_description']);
        $node_wrapper->field_shl_description1->set($val);
    }
    //内容提要
    if(isset($data['abstracts_xml'])) {
        $val = is_array($data['abstracts_xml']) ? $data['abstracts_xml'] : array($data['abstracts_xml']);
        $node_wrapper->field_dct_abstract->set($val);
    }
    //载体形态附注
    if(isset($data['notesPertainingToPhysicalDescription'])) {
        $val = is_array($data['notesPertainingToPhysicalDescription']) ? $data['notesPertainingToPhysicalDescription'] : array($data['notesPertainingToPhysicalDescription']);
        $node_wrapper->field_shl_description2->set($val);
    }
    //个人捐赠者
    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
   //权限
    if(isset($data['Rights'])) {
        $val = is_array($data['Rights']) ? $data['Rights'] : array($data['Rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!empty($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }

    if(isset($data['RelatedResource']))
    {
        //相关资源名
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        //相关资源索取号
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }
    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }
    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}
/**
 * 插入签名本数据
 */
function import_history_zhengjian_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['TitleAndCreator']['dc_Title'];
    $node->type = 'qianmingben';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;
    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=strtotime($data['last_update_time']);
    }
    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
    //ISBN
    if(isset($data['iSBN'])) {
        $val = is_array($data['iSBN']) ? $data['iSBN'] : array($data['iSBN']);
        $node_wrapper->field_shl_isbn->set($val);
    }
    //并列题名
    if(isset($data['dcterms_alternative'])) {
        $val = is_array($data['dcterms_alternative']) ? $data['dcterms_alternative'] : array($data['dcterms_alternative']);
        $node_wrapper->field_dct_alternative->set($val);
    }
    //副题名
    if(isset($data['second'])) {
        $val = is_array($data['second']) ? $data['second'] : array($data['second']);
        $node_wrapper->field_shl_subtitle->set($val);
    }
    //其他题名
    if(isset($data['other'])) {
        $val = is_array($data['other']) ? $data['other'] : array($data['other']);
        $node_wrapper->field_shl_varianttitle->set($val);
    }
    //分辑名
    if(isset($data['dcterms_hasPart'])) {
        $val = is_array($data['dcterms_hasPart']) ? $data['dcterms_hasPart'] : array($data['dcterms_hasPart']);
        $node_wrapper->field_dct_haspart->set($val);
    }
    //个人作者
    $node_wrapper->field_dc_creator->set(array($data['Personalandrole']['personal']));
    // 责任方式
    if(isset($data['Personalandrole']['personalrole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['Personalandrole']['personalrole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role->set(array($tt));
    }
    // 团体责任者
    if(isset($data['Corporateandrole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $val = is_array($data['Corporateandrole']['corporate']) ? $data['Corporateandrole']['corporate'] : array($data['Corporateandrole']['corporate']);
        $node_wrapper->field_dc_creator4->set($val);

        $tt = null;
        foreach($tree as $term) {
            if($term->name == $data['Corporateandrole']['corporaterole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role1->set(array($tt));
    }
    // 其他责任者
    if(isset($data['ContributorRole'])) {
        $val = is_array($data['ContributorRole']['contributor']) ? $data['ContributorRole']['contributor'] : array($data['ContributorRole']['contributor']);
        $node_wrapper->field_contributor->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['ContributorRole']['contributorAndRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_contributorrole->set(array($tt));
    }
    //版本
    if(isset($data['edition'])) {
        $val = is_array($data['edition']) ? $data['edition'] : array($data['edition']);
        $node_wrapper->field_banben->set($val);
    }
    // 出版社
    if(isset($data['Publication'])) {
        $val = is_array($data['Publication']['placeOfPublication']) ? $data['Publication']['placeOfPublication'] : array($data['Publication']['placeOfPublication']);
        $node_wrapper->field__di_useapi->set($val);
        $val = is_array($data['Publication']['publisher']) ? $data['Publication']['publisher'] : array($data['Publication']['publisher']);
        $node_wrapper->field_dc_publisher->set($val);

        if(isset($data['Publication']['issued'])) {
            $str = str_replace('[?]', '', $data['Publication']['issued']);
            $str = str_replace('[不详]', '', $data['Publication']['issued']);
            $str = str_replace('不详', '', $data['Publication']['issued']);
            $str = str_replace('日', '.', $data['Publication']['issued']);
            $str = str_replace('年', '.', $data['Publication']['issued']);
            $str = str_replace('月', '.', $data['Publication']['issued']);
            $str = str_replace('.', '-', $data['Publication']['issued']);
            $ta = explode('-', $str);
            $ta = array_map(function($item) {
                return trim($item);
            }, $ta);
            if(!isset($ta[1])) $ta[1] = '01';
            if(!isset($ta[2])) $ta[2] = '01';
            $str = implode('-', $ta);
            $str .= ' 00:00:00';
            if(strtotime($str)) {
                $node_wrapper->field_cb_dc_date->set(array(array('value' => $str, 'value2' => $str)));
            }
        }
    }
    //数量及单位
    $node_wrapper->field_bf_extent->set($data['amount']);
     //尺寸
    $node_wrapper->field_bf_dimensions->set($data['size']);

    //装订方式
    if(isset($data['descriptionOfBinding'])) {
        $tax = taxonomy_vocabulary_machine_name_load('taxonomy_binding_type');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['descriptionOfBinding']) ? $data['descriptionOfBinding'] : array($data['descriptionOfBinding']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
             $node_wrapper->field_bjzdxs->set($tt);
    }
    //中国图书馆图书分类法分类号
    if(isset($data['cLC'])) {
        $val = is_array($data['cLC']) ? $data['cLC'] : array($data['cLC']);
        $node_wrapper->field_tsgflh->set($val);
    }
    //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
      // 题记
    if(isset($data['Inscription'])) {
        //个人下款名
        $val = is_array($data['Inscription']['personalInscriber']) ? $data['Publication']['personalInscriber'] : array($data['Publication']['personalInscriber']);
        $node_wrapper->field_dc_creator5->set($val);
        //团体下款名
        // $val = is_array($data['Inscription']['corporateInscriber']) ? $data['Publication']['corporateInscriber'] : array($data['Publication']['corporateInscriber']);
        // $node_wrapper->field_dc_creator1->set($val);
        $node_wrapper->field_dc_creator1->set($data['Inscription']['corporateInscriber']);
        //题记位置
        $val = is_array($data['Inscription']['locationOfInscriptions']) ? $data['Publication']['locationOfInscriptions'] : array($data['Publication']['locationOfInscriptions']);
        $node_wrapper->field_shl_locationofinscription->set($val);
         //题记全文
         $val = is_array($data['Inscription']['inscriptions']) ? $data['Publication']['inscriptions'] : array($data['Publication']['inscriptions']);
         $node_wrapper->field_shl_contentofinscription->set($val);
         //题记卷册
          $val = is_array($data['Inscription']['numberOfInscriptions']) ? $data['Publication']['numberOfInscriptions'] : array($data['Publication']['numberOfInscriptions']);
          $node_wrapper->field_shl_dimensionofinscriptio->set($val);
        //题记时间
        if(isset($data['Inscription']['modified'])) {
            $str = str_replace('[?]', '', $data['Inscription']['modified']);
            $str = str_replace('[不详]', '', $data['Inscription']['modified']);
            $str = str_replace('不详', '', $data['Inscription']['modified']);
            $str = str_replace('日', '.', $data['Inscription']['modified']);
            $str = str_replace('年', '.', $data['Inscription']['modified']);
            $str = str_replace('月', '.', $data['Inscription']['modified']);
            $str = str_replace('.', '-', $data['Inscription']['modified']);
            $ta = explode('-', $str);
            $ta = array_map(function($item) {
                return trim($item);
            }, $ta);
            if(!isset($ta[1])) $ta[1] = '01';
            if(!isset($ta[2])) $ta[2] = '01';
            $str = implode('-', $ta);
            $str .= ' 00:00:00';
            if(strtotime($str)) {
                $node_wrapper->field_dc_date->set(array(array('value' => $str, 'value2' => $str)));
            }
        }
    }
    // 印章
    if(isset($data['Seal'])) {
        //印章类型
        if(isset($data['Seal']['typeOfSeal'])) {
            $tax = taxonomy_vocabulary_machine_name_load('yinzhangleixing');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['Seal']['typeOfSeal']) ? $data['Seal']['typeOfSeal'] : array($data['Seal']['typeOfSeal']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }

            }
            if(!empty($tt))
                 $node_wrapper->field_shl_typeofseal->set($tt);
        }
        //印章印文
        $val = is_array($data['Seal']['inscriptionsOfSeal']) ? $data['Seal']['inscriptionsOfSeal'] : array($data['Seal']['inscriptionsOfSeal']);
        $node_wrapper->field_shl_inscriptionsofseal->set($val);
        //印文类型
        if(isset($data['Seal']['inscriptionsTypeOfSeal'])) {
            $tax = taxonomy_vocabulary_machine_name_load('yinwenleixing');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['Seal']['inscriptionsTypeOfSeal']) ? $data['Seal']['inscriptionsTypeOfSeal'] : array($data['Seal']['inscriptionsTypeOfSeal']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }

            }
            if(!empty($tt))
                 $node_wrapper->field_shl_typeofinscription->set($tt);
        }
         //藏书章所有人
         if(isset($data['Seal']['ownerOfSeal']))
         {
             $val = is_array($data['Seal']['ownerOfSeal']) ? $data['Seal']['ownerOfSeal'] : array($data['Seal']['ownerOfSeal']);
             $node_wrapper->field_csz_shl_ownerofseal->set($val);
         }


    }
     // 藏书票
     if(isset($data['Stamp'])) {
        //藏书票文字
        if(isset($data['Stamp']['inscriptionsOfStamp']))
        {
            $val = is_array($data['Stamp']['inscriptionsOfStamp']) ? $data['Stamp']['inscriptionsOfStamp'] : array($data['Stamp']['inscriptionsOfStamp']);
            $node_wrapper->field_csz_shl_ownerofseal->set($val);
        }
       //藏书票图案描述
       if(isset($data['Stamp']['descriptionOfStamp']))
       {
            $val = is_array($data['Stamp']['descriptionOfStamp']) ? $data['Stamp']['descriptionOfStamp'] : array($data['Stamp']['descriptionOfStamp']);
            $node_wrapper->field_csp_shl_description->set($val);
        }
        //藏书票尺寸
        if(isset($data['Stamp']['sizeOfStamp']))
        {
            $val = is_array($data['Stamp']['sizeOfStamp']) ? $data['Stamp']['sizeOfStamp'] : array($data['Stamp']['sizeOfStamp']);
            $node_wrapper->field_shl_sizeofbookplate->set($val);
        }
    }
    // 获取方式
    if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_obtainedby->set($tt);
        }

    }
    //语种
    if(!isset($data['language']) || $data['language'] == '中文') $data['language'] = '汉语';
    if(isset($data['language'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);

        $tt = array();
        $val = is_array($data['language']) ? $data['language'] : array($data['language']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }
      //翻译作者原语种
      if(!isset($data['languageOfOriginal']) || $data['languageOfOriginal'] == '中文') $data['languageOfOriginal'] = '汉语';
      if(isset($data['languageOfOriginal'])) {
          $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
          $tree = taxonomy_get_tree($tax->vid);

          $tt = array();
          $val = is_array($data['languageOfOriginal']) ? $data['languageOfOriginal'] : array($data['languageOfOriginal']);
          foreach($tree as $term) {
              foreach ($val as $item) {
                  if($term->name == $item) {
                      array_push($tt,taxonomy_term_load($term->tid)) ;
                  }
              }
          }
          if(!empty($tt))
              $node_wrapper->field_qmb_dc_language->set($tt);
      }
    //个人捐赠者
    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
   //权限
    if(isset($data['Rights'])) {
        $val = is_array($data['Rights']) ? $data['Rights'] : array($data['Rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!empty($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }
    //一般性附注
    if(isset($data['dc_description'])) {
        $val = is_array($data['dc_description']) ? $data['dc_description'] : array($data['dc_description']);
        $node_wrapper->field_shl_description1->set($val);
    }

    if(isset($data['RelatedResource']))
    {
        //相关资源名
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        //相关资源索取号
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }
    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }
    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}
/**
 * 插入创作手稿数据
 */
function import_history_chuangzuoshougao_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'chuangzuoshougao';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;
    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=strtotime($data['last_update_time']);
    }
    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
     //并列题名
     if(isset($data['dcterms_alternative'])) {
        $val = is_array($data['dcterms_alternative']) ? $data['dcterms_alternative'] : array($data['dcterms_alternative']);
        $node_wrapper->field_dct_alternative->set($val);
    }
    //副题名
    if(isset($data['second'])) {
        $val = is_array($data['second']) ? $data['second'] : array($data['second']);
        $node_wrapper->field_shl_subtitle->set($val);
    }
     //统一题名
     if(isset($data['uniform'])) {
        $val = is_array($data['uniform']) ? $data['uniform'] : array($data['uniform']);
        $node_wrapper->field_czsg_dc_title->set($val);
    }
    //其他题名
    if(isset($data['other'])) {
        $val = is_array($data['other']) ? $data['other'] : array($data['other']);
        $node_wrapper->field_shl_varianttitle->set($val);
    }
    //初次发表名
    if(isset($data['firstIssued'])) {
        $val = is_array($data['firstIssued']) ? $data['firstIssued'] : array($data['firstIssued']);
        $node_wrapper->field_ccfb_dc_title->set($val);
    }

    $node_wrapper->field_dc_creator->set(array($data['RoleOfPersonal']['personal']));
    // 责任方式
    if(isset($data['RoleOfPersonal']['personalRole'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['RoleOfPersonal']['personalRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role->set(array($tt));
    }

     // 个人其他责任者
     if(isset($data['Personalcontributor'])) {
        $val = is_array($data['Personalcontributor']['contributorpersonal']) ? $data['Personalcontributor']['contributorpersonal'] : array($data['Personalcontributor']['contributorpersonal']);
        $node_wrapper->field_bf_agent->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['Personalcontributor']['contributorpersonalRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role2->set(array($tt));
    }
     // 团体其他责任者
     if(isset($data['Corporatecontributor'])) {
        $val = is_array($data['Corporatecontributor']['contributorcorporate']) ? $data['Corporatecontributor']['contributorcorporate'] : array($data['Corporatecontributor']['contributorcorporate']);
        $node_wrapper->field_bf_agent1->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = null;
        foreach($tree as $term) {
            if($term->name == $data['Corporatecontributor']['contributor_corporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role3->set(array($tt));
    }
    // 获取方式
    if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_shl_obtainedby->set($tt);
    }
    //版本
   if(isset($data['edition'])) {
    $val = is_array($data['edition']) ? $data['edition'] : array($data['edition']);
    $node_wrapper->field_banben->set($val);
    }
    //创作地
    if(isset($data['locationOfCreation'])) {
        $val = is_array($data['locationOfCreation']) ? $data['locationOfCreation'] : array($data['locationOfCreation']);
        $node_wrapper->field__di_useapi->set($val);
    }
    //创作时间
    if(isset($data['created'])) {
        $val = is_array($data['created']) ? $data['created'] : array($data['created']);
        $node_wrapper->field_created->set($val);
    }
    //数量及单位
    if(isset($data['amount'])) {
        $val = is_array($data['amount']) ? $data['amount'] : array($data['amount']);
        $node_wrapper->field_bf_extent->set($val);
    }
    //尺寸
    if(isset($data['size'])) {
        $val = is_array($data['size']) ? $data['size'] : array($data['size']);
        $node_wrapper->field_bf_dimensions->set($val);
    }
     //书写工具
     if(isset($data['ImplementAndcolor']['implement'])) {
        $tax = taxonomy_vocabulary_machine_name_load('shuxiegongju');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['ImplementAndcolor']['implement']) ? $data['ImplementAndcolor']['implement'] : array($data['ImplementAndcolor']['implement']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_shl_writingtool->set($tt);
    }
    //色泽
    if(isset($data['ImplementAndcolor']['color'])) {
        $tax = taxonomy_vocabulary_machine_name_load('seze');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['ImplementAndcolor']['color']) ? $data['ImplementAndcolor']['color'] : array($data['ImplementAndcolor']['color']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_shl_writingcolor->set($tt);
    }

    //一般性附注
    if(isset($data['description'])) {
        $val = is_array($data['description']) ? $data['description'] : array($data['description']);
        $node_wrapper->field_shl_description1->set($val);
    }
    //初次发表处
    if(isset($data['placeOfInitialIssued'])) {
        $val = is_array($data['placeOfInitialIssued']) ? $data['placeOfInitialIssued'] : array($data['placeOfInitialIssued']);
        $node_wrapper->field_ccsg_dc_publisher->set($val);
    }
    //初次发表时间
    if(isset($data['issued'])) {
        $val = is_array($data['issued']) ? $data['issued'] : array($data['issued']);
        $node_wrapper->field_ccfb_time->set($val);
    }
     //出版形态附注
     if(isset($data['notesToPublication'])) {
        $val = is_array($data['notesToPublication']) ? $data['notesToPublication'] : array($data['notesToPublication']);
        $node_wrapper->field_cbzt_dc_publisher->set($val);
    }
     //载体形态附注
     if(isset($data['notesPertainingToPhysicalDescription'])) {
        $val = is_array($data['notesPertainingToPhysicalDescription']) ? $data['notesPertainingToPhysicalDescription'] : array($data['notesPertainingToPhysicalDescription']);
        $node_wrapper->field_shl_description2->set($val);
    }
    //内容提要
    if(isset($data['abstracts_xml'])) {
        $val = is_array($data['abstracts_xml']) ? $data['abstracts_xml'] : array($data['abstracts_xml']);
        $node_wrapper->field_dct_abstract->set($val);
    }
    //个人名称
    if(isset($data['nameOfPerson'])) {
        $val = is_array($data['nameOfPerson']) ? $data['nameOfPerson'] : array($data['nameOfPerson']);
        $node_wrapper->field_rj_dc_creator->set($val);
    }


    if(isset($data['Corporatesubject'])) {
        //团体名称
        if(isset($data['Corporatesubject']['nameOfCorporation'])) {
            $val = is_array($data['Corporatesubject']['nameOfCorporation']) ? $data['Corporatesubject']['nameOfCorporation'] : array($data['Corporatesubject']['nameOfCorporation']);
            $node_wrapper->field_dc_tt_subject->set($val);
        }
        //会议届次
        if(isset($data['Corporatesubject']['numberOfConference'])) {
            $val = is_array($data['Corporatesubject']['numberOfConference']) ? $data['Corporatesubject']['numberOfConference'] : array($data['Corporatesubject']['numberOfConference']);
            $node_wrapper->field_bjhyjc->set($val);
        }
        //会议地点
        if(isset($data['Corporatesubject']['placeOfConference'])) {
            $val = is_array($data['Corporatesubject']['placeOfConference']) ? $data['Corporatesubject']['placeOfConference'] : array($data['Corporatesubject']['placeOfConference']);
            $node_wrapper->field_shl_place->set($val);
        }

        // 会议时间
        if(isset($data['Corporatesubject']['dateOfConference'])) {
            $str = str_replace('[?]', '', $data['Corporatesubject']['dateOfConference']);
            $str = str_replace('[不详]', '', $data['Corporatesubject']['dateOfConference']);
            $str = str_replace('不详', '',  $data['Corporatesubject']['dateOfConference']);
            $str = str_replace('日', '.',  $data['Corporatesubject']['dateOfConference']);
            $str = str_replace('年', '.',  $data['Corporatesubject']['dateOfConference']);
            $str = str_replace('月', '.',  $data['Corporatesubject']['dateOfConference']);
            $str = str_replace('.', '-', $data['Corporatesubject']['dateOfConference']);
            $ta = explode('-', $str);
            $ta = array_map(function($item) {
                return trim($item);
            }, $ta);
            if(!isset($ta[1])) $ta[1] = '01';
            if(!isset($ta[2])) $ta[2] = '01';
            $str = implode('-', $ta);
            $str .= ' 00:00:00';
            if(strtotime($str)) {
                $node_wrapper->field_shl_temporalvalues->set(array(array('value' => $str, 'value2' => $str)));
            }
        }
    }
    //重要事件主题
    if(isset($data['subjectOfEvents'])) {
        $val = is_array($data['subjectOfEvents']) ? $data['subjectOfEvents'] : array($data['subjectOfEvents']);
        $node_wrapper->field_lssj_dc_subject->set($val);
    }
    //学科名称主题
    if(isset($data['SubjectofScience'])) {
        //学科名称
        if(isset($data['SubjectofScience']['nameOfSubject']))
         {
            $val = is_array($data['SubjectofScience']['nameOfSubject']) ? $data['SubjectofScience']['nameOfSubject'] : array($data['SubjectofScience']['nameOfSubject']);
            $node_wrapper->field_xkmc_dc_title->set($val);
        }
        //学科主题复分
        if(isset($data['SubjectofScience']['topicOfSubject'])) {
            $val = is_array($data['SubjectofScience']['topicOfSubject']) ? $data['SubjectofScience']['topicOfSubject'] : array($data['SubjectofScience']['topicOfSubject']);
            $node_wrapper->field_topicofsubject->set($val);
        }
        //地区复分
        if(isset($data['SubjectofScience']['subjectOfRegion'])) {
            $val = is_array($data['SubjectofScience']['subjectOfRegion']) ? $data['SubjectofScience']['subjectOfRegion'] : array($data['SubjectofScience']['subjectOfRegion']);
            $node_wrapper->field_dqff_shl_place->set($val);
        }
        //时间复分
        if(isset($data['SubjectofScience']['subjectOfTime'])) {
            $val = is_array($data['SubjectofScience']['subjectOfTime']) ? $data['SubjectofScience']['subjectOfTime'] : array($data['SubjectofScience']['subjectOfTime']);
            $node_wrapper->field_sjff_shl_temporal->set($val);
        }

    }
    //地名主题
    if(isset($data['place'])) {
        $val = is_array($data['place']) ? $data['place'] : array($data['place']);
        $node_wrapper->field_dmzt_dc_subject->set($val);
    }
     // 文献类型
    if(isset($data['type'])) {
        $tax = taxonomy_vocabulary_machine_name_load('czsgwxlx');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['type']) ? $data['type'] : array($data['type']);
        foreach($tree as $term) {
            foreach ($val as $item) {
             if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
            $node_wrapper->field_field_czsg_dc_type->set($tt);
    }
     //关键词
     if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
    //个人捐赠者
    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
    // 装订方式
    if(isset($data['descriptionOfBinding'])) {
        $tax = taxonomy_vocabulary_machine_name_load('taxonomy_binding_type');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['descriptionOfBinding']) ? $data['descriptionOfBinding'] : array($data['descriptionOfBinding']);
        foreach($tree as $term) {
            foreach ($val as $item) {
             if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }

        }
        if(!empty($tt))
            $node_wrapper->field_bjzdxs->set($tt);
    }

    // 语种
    if(!isset($data['language']) || $data['language'] == '中文') $data['language'] = '汉语';
    if(isset($data['language'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['language']) ? $data['language'] : array($data['language']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }
    // 翻译中间原语种
    if(!isset($data['languageOfOrigin']) || $data['languageOfOrigin'] == '中文') $data['languageOfOrigin'] = '汉语';
    if(isset($data['languageOfOrigin'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['languageOfOrigin']) ? $data['languageOfOrigin'] : array($data['languageOfOrigin']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_qmb_dc_language->set($tt);
    }

    //权限
    if(isset($data['dc_rights'])) {
        $val = is_array($data['dc_rights']) ? $data['dc_rights'] : array($data['dc_rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }
    if(isset($data['RelatedResource']))
    {
        //相关资源名
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        //相关资源索取号
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }
    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }

    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}
/**
 * 插入信函数据
 */
function import_history_xinhan_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'xinhan';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;
    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=strtotime($data['last_update_time']);
    }
    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);
    //标识符
    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    //索取号
    $node_wrapper->field_dct_identifier->set($data['callNo']);
    //排架号
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);

    //个人下款名
    if(isset($data['creatorPersonal'])) {
        $val = is_array($data['creatorPersonal']) ? $data['creatorPersonal'] : array($data['creatorPersonal']);
        $node_wrapper->field_dc_creator->set($val);
    }
    //团体下款名
    if(isset($data['creatorCorporate'])) {
        $val = is_array($data['creatorCorporate']) ? $data['creatorCorporate'] : array($data['creatorCorporate']);
        $node_wrapper->field_dc_creator1->set($val);
    }
     //文献类型
     if(isset($data['type'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_leix');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['type']) ? $data['type'] : array($data['type']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_field_xh_dc_type->set($tt);
    }
    // 获取方式
    if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_shl_obtainedby->set($tt);
    }
   //数量及单位
   if(isset($data['amount'])) {
    $val = is_array($data['amount']) ? $data['amount'] : array($data['amount']);
    $node_wrapper->field_bf_extent->set($val);
    }
    //尺寸
    if(isset($data['size'])) {
        $val = is_array($data['size']) ? $data['size'] : array($data['size']);
        $node_wrapper->field_bf_dimensions->set($val);
    }
     //落款时间
     if(isset($data['created'])) {
        $val = is_array($data['created']) ? $data['created'] : array($data['created']);
        $node_wrapper->field_created->set($val);
    }
    // 发送邮戳日期
    if(isset($data['postmarkFrom'])) {
        $str = str_replace('[?]', '', $data['postmarkFrom']);
        $str = str_replace('[不详]', '', $data['postmarkFrom']);
        $str = str_replace('不详', '',  $data['postmarkFrom']);
        $str = str_replace('日', '.',  $data['postmarkFrom']);
        $str = str_replace('年', '.', $data['postmarkFrom']);
        $str = str_replace('月', '.',  $data['postmarkFrom']);
        $str = str_replace('.', '-', $data['postmarkFrom']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_shl_senddate->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
    // 接收邮戳日期
    if(isset($data['postmarkTo'])) {
        $str = str_replace('[?]', '', $data['postmarkTo']);
        $str = str_replace('[不详]', '', $data['postmarkTo']);
        $str = str_replace('不详', '',  $data['postmarkTo']);
        $str = str_replace('日', '.',  $data['postmarkTo']);
        $str = str_replace('年', '.', $data['postmarkTo']);
        $str = str_replace('月', '.',  $data['postmarkTo']);
        $str = str_replace('.', '-', $data['postmarkTo']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_shl_acceptdate->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
    //创作地
    if(isset($data['locationOfCreation'])) {
        $val = is_array($data['locationOfCreation']) ? $data['locationOfCreation'] : array($data['locationOfCreation']);
        $node_wrapper->field__di_useapi->set($val);
    }

    //历史事件主题
    if(isset($data['subjectOfEvents'])) {
        $val = is_array($data['subjectOfEvents']) ? $data['subjectOfEvents'] : array($data['subjectOfEvents']);
        $node_wrapper->field_lssj_dc_subject->set($val);
    }
    //地名主题
    if(isset($data['place'])) {
        $val = is_array($data['place']) ? $data['place'] : array($data['place']);
        $node_wrapper->field_dmzt_dc_subject->set($val);
    }
     //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
    //一般性附注
    if(isset($data['annotation'])) {
        $val = is_array($data['annotation']) ? $data['annotation'] : array($data['annotation']);
        $node_wrapper->field_shl_description1->set($val);
    }
     //内容提要
     if(isset($data['abstracts_xml'])) {
        $val = is_array($data['abstracts_xml']) ? $data['abstracts_xml'] : array($data['abstracts_xml']);
        $node_wrapper->field_dct_abstract->set($val);
    }
     //信函完整性附注
     if(isset($data['integralityAnnotation'])) {
        $val = is_array($data['integralityAnnotation']) ? $data['integralityAnnotation'] : array($data['integralityAnnotation']);
        $node_wrapper->field_integralityannotation->set($val);
    }
    //载体形态附注
    if(isset($data['notesPertainingToPhysicalDescription'])) {
        $val = is_array($data['notesPertainingToPhysicalDescription']) ? $data['notesPertainingToPhysicalDescription'] : array($data['notesPertainingToPhysicalDescription']);
        $node_wrapper->field_shl_description2->set($val);
    }
    //信纸风格特征
    if(isset($data['styleOfLetter'])) {
        $val = is_array($data['styleOfLetter']) ? $data['styleOfLetter'] : array($data['styleOfLetter']);
        $node_wrapper->field_shl_styleofpaper->set($val);
    }
     //信封个人收信者
     if(isset($data['personalAccepter'])) {
        $val = is_array($data['personalAccepter']) ? $data['personalAccepter'] : array($data['personalAccepter']);
        $node_wrapper->field_shl_acceptor->set($val);
    }
     //信封个人寄者
     if(isset($data['personalSender'])) {
        $val = is_array($data['personalSender']) ? $data['personalSender'] : array($data['personalSender']);
        $node_wrapper->field_shl_sender->set($val);
    }

     //信封团体寄信者
     if(isset($data['corporateSender'])) {
        $val = is_array($data['corporateSender']) ? $data['corporateSender'] : array($data['corporateSender']);
        $node_wrapper->field_xfttjxz->set($val);
    }
     //信封团体收信者
     if(isset($data['corporateAccepter'])) {
        $val = is_array($data['corporateAccepter']) ? $data['corporateAccepter'] : array($data['corporateAccepter']);
        $node_wrapper->field_tt_shl_acceptor->set($val);
    }
    //寄信地
    if(isset($data['placeOfMailing'])) {
        $val = is_array($data['placeOfMailing']) ? $data['placeOfMailing'] : array($data['placeOfMailing']);
        $node_wrapper->field_jixindi_useapi->set($val);
    }
    //寄信地地址栏
    if(isset($data['addressOfSender'])) {
        $val = is_array($data['addressOfSender']) ? $data['addressOfSender'] : array($data['addressOfSender']);
        $node_wrapper->field__jxddizhilan->set($val);
    }
    //寄信地邮编
    if(isset($data['postalcodeOfSending'])) {
        $val = is_array($data['postalcodeOfSending']) ? $data['postalcodeOfSending'] : array($data['postalcodeOfSending']);
        $node_wrapper->field_jxdyb->set($val);
    }
    //收信地
    if(isset($data['placeOfAccept'])) {
        $val = is_array($data['placeOfAccept']) ? $data['placeOfAccept'] : array($data['placeOfAccept']);
        $node_wrapper->field_sxd_useapi->set($val);
    }
    //收信地地址栏
    if(isset($data['addressOfAcceptor'])) {
        $val = is_array($data['addressOfAcceptor']) ? $data['addressOfAcceptor'] : array($data['addressOfAcceptor']);
        $node_wrapper->field__sxddizhil->set($val);
    }
    //收信地邮编
    if(isset($data['postalcodeOfAccepting'])) {
        $val = is_array($data['postalcodeOfAccepting']) ? $data['postalcodeOfAccepting'] : array($data['postalcodeOfAccepting']);
        $node_wrapper->field_sxdyb->set($val);
    }

    // 信纸语种
    if(!isset($data['languageOfLetter']) || $data['languageOfLetter'] == '中文') $data['languageOfLetter'] = '汉语';
    if(isset($data['languageOfLetter'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['languageOfLetter']) ? $data['languageOfLetter'] : array($data['languageOfLetter']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }
    // 信封语种
    if(!isset($data['languageOfEnvelope']) || $data['languageOfEnvelope'] == '中文') $data['languageOfEnvelope'] = '汉语';
    if(isset($data['languageOfEnvelope'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['languageOfEnvelope']) ? $data['languageOfEnvelope'] : array($data['languageOfEnvelope']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_xz_dc_language->set($tt);
    }
     //个人捐赠者
     if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }

    //权限
    if(isset($data['dc_rights'])) {
        $val = is_array($data['dc_rights']) ? $data['dc_rights'] : array($data['dc_rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }
    if(isset($data['RelatedResource']))
    {
         //相关资源名
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        //相关资源索取号
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }
    //个人名称
    if(isset($data['nameOfPerson'])) {
        $val = is_array($data['nameOfPerson']) ? $data['nameOfPerson'] : array($data['nameOfPerson']);
        $node_wrapper->field_dc_subjectbj->set($val);
    }
    //团体名称主题
    if(isset($data['nameOfCorporation'])) {
        $val = is_array($data['nameOfCorporation']) ? $data['nameOfCorporation'] : array($data['nameOfCorporation']);
        $node_wrapper->field_dc_tt_subject->set($val);
    }
     //信纸书写工具
     if(isset($data['Letterimplent']['implementOfLetter'])) {
        $tax = taxonomy_vocabulary_machine_name_load('shuxiegongju');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['Letterimplent']['implementOfLetter']) ? $data['Letterimplent']['implementOfLetter'] : array($data['Letterimplent']['implementOfLetter']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_shl_writingtool->set($tt);
    }
    //信纸书写颜色
    if(isset($data['Letterimplent']['colorOfLetter'])) {
        $tax = taxonomy_vocabulary_machine_name_load('seze');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['Letterimplent']['colorOfLetter']) ? $data['Letterimplent']['colorOfLetter'] : array($data['Letterimplent']['colorOfLetter']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
             $node_wrapper->field_shl_writingcolor->set($tt);
    }
    //信封书写工具
    if(isset($data['Envelopimplent']['implementOfEnvelope'])) {
        $tax = taxonomy_vocabulary_machine_name_load('shuxiegongju');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['Envelopimplent']['implementOfEnvelope']) ? $data['Envelopimplent']['implementOfEnvelope'] : array($data['Envelopimplent']['implementOfEnvelope']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_xf_shl_writingtool->set($tt);
    }
    //信封书写颜色
    if(isset($data['Envelopimplent']['colorOfEnvelope'])) {
        $tax = taxonomy_vocabulary_machine_name_load('seze');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['Envelopimplent']['colorOfEnvelope']) ? $data['Envelopimplent']['colorOfEnvelope'] : array($data['Envelopimplent']['colorOfEnvelope']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_xf_shl_wirtingcolor->set($tt);
    }
    //个人上款名
    if(isset($data['Zhubiaopersonal']['personal'])) {
        $val = is_array($data['Zhubiaopersonal']['personal']) ? $data['Zhubiaopersonal']['personal'] : array($data['Zhubiaopersonal']['personal']);
        $node_wrapper->field_dc_contributor_ming->set($val);
    }
    //团体上款名
    if(isset($data['contributorCorporate'])) {
        $val = is_array($data['contributorCorporate']) ? $data['contributorCorporate']: array($data['contributorCorporate']);
        $node_wrapper->field_dc_contributor_ming1->set($val);
    }
    if(isset($data['selfUri'])) {
       // $val = is_array($data['selfUri']) ? $data['selfUri'] : array($data['selfUri']);
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }

    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}
/**
 * 插入书画篆刻数据
 */
function import_history_shuhuazhuanke_data_insert() {
    global $user;
    $data = $_POST;

    $node = new stdClass();
    $node->title = $data['dc_Title'];
    $node->type = 'shuhuazhuanke';
    node_object_prepare($node);
    $node->language = LANGUAGE_NONE;
    if(isset($data['input_user']))
    {
        $node->uid=intval($data['input_user']);
    }
    if(isset($data['created_time']))
    {
        $node->date=$data['created_time'];
    }

    if(isset($data['last_update_time']))
    {
        $node->changing=strtotime($data['last_update_time']);
    }
    $node->promote = 0;
    $node->comment = 0;
    $node->workbench_access = 7;
    $node->workbench_moderation_state_new = 'published';
    $node = node_submit($node);
    node_save($node);

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $node_wrapper->field_biaoshifu->set($data['Identifier']);
    $node_wrapper->field_shl_shelfmark->set($data['shelvingControlNumber']);
    //索取号
    if(isset($data['callNo'])) {
        $val = is_array($data['callNo']) ? $data['callNo'] : array($data['callNo']);
        $node_wrapper->field_dct_identifier->set($val);
    }
    //副题名
    if(isset($data['alternative'])) {
        $val = is_array($data['alternative']) ? $data['alternative'] : array($data['alternative']);
        $node_wrapper->field_shl_subtitle->set($val);
    }

    // 个人下款名
    if(isset($data['PersonalCreator'])) {
        $val = is_array($data['PersonalCreator']['creatorPersonal']) ? $data['PersonalCreator']['creatorPersonal'] : array($data['PersonalCreator']['creatorPersonal']);
        $node_wrapper->field_dc_creator->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['PersonalCreator']['personalRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role->set(array($tt));
    }
    //团体下款名
    // 责任方式
    if(isset($data['CoporateCreator'])) {
        $val = is_array($data['CoporateCreator']['creatorCorporate']) ? $data['CoporateCreator']['creatorCorporate'] : array($data['CoporateCreator']['creatorCorporate']);
        $node_wrapper->field_dc_creator1->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = null;
        foreach($tree as $term) {
            if($term->name == $data['CoporateCreator']['corporateRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role1->set(array($tt));
    }
     //个人其他责任者
    // 责任方式
    if(isset($data['PersonalcontributorAndRole'])) {
        $val = is_array($data['PersonalcontributorAndRole']['personalContributor']) ? $data['PersonalcontributorAndRole']['personalContributor'] : array($data['PersonalcontributorAndRole']['personalContributor']);
        $node_wrapper->field_bf_agent->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['PersonalcontributorAndRole']['personalContributorRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role2->set(array($tt));
    }
    //团体其他责任者
    // 责任方式
    if(isset($data['CorporatecontributorAndRole'])) {
        $val = is_array($data['CorporatecontributorAndRole']['corporateContributor']) ? $data['CorporatecontributorAndRole']['corporateContributor'] : array($data['CorporatecontributorAndRole']['corporateContributor']);
        $node_wrapper->field_bf_agent1->set($val);
        $tax = taxonomy_vocabulary_machine_name_load('_fangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['CorporatecontributorAndRole']['corporateContributorRole']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!is_null($tt))
            $node_wrapper->field_bf_role3->set(array($tt));
    }
    //下款说明
    if(isset($data["Subscriptnote"]))
    {
         //个人下款名
        $val = is_array($data['Subscriptnote']['creatorPersonal']) ? $data['Subscriptnote']['creatorPersonal'] : array($data['Subscriptnote']['creatorPersonal']);
        $node_wrapper->field_dc_creator2->set($val);
         //下款内容
        $val = is_array($data['Subscriptnote']['subscript']) ? $data['Subscriptnote']['subscript'] : array($data['Subscriptnote']['subscript']);
        $node_wrapper->field_shl_description->set($val);
        //下款说明_印章类型
        if(isset($data['Subscriptnote']['typeOfSeal'])) {
            $tax = taxonomy_vocabulary_machine_name_load('yinzhangleixing');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['Subscriptnote']['typeOfSeal']) ? $data['Subscriptnote']['typeOfSeal'] : array($data['Subscriptnote']['typeOfSeal']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
                 $node_wrapper->field_xk_typeofseal->set($tt);
        }
        //下款说明_印章位置
        $val = is_array($data['Subscriptnote']['locationOfSeal']) ? $data['Subscriptnote']['locationOfSeal'] : array($data['Subscriptnote']['locationOfSeal']);
        $node_wrapper->field_xk_locationofseal->set($val);
        //下款说明_印章印文
        $val = is_array($data['Subscriptnote']['inscriptionsOfSeal']) ? $data['Subscriptnote']['inscriptionsOfSeal'] : array($data['Subscriptnote']['inscriptionsOfSeal']);
        $node_wrapper->field_xk_inscriptionsofsea->set($val);
         //下款说明_印文类型
         if(isset($data['Subscriptnote']['typeOfSealInscriptions'])) {
            $tax = taxonomy_vocabulary_machine_name_load('yinwenleixing');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['Subscriptnote']['typeOfSealInscriptions']) ? $data['Subscriptnote']['typeOfSealInscriptions'] : array($data['Subscriptnote']['typeOfSealInscriptions']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
                 $node_wrapper->field_xk_typeofsealinscriptions->set($tt);
        }
        //题记者
        $val = is_array($data['Subscriptnote']['inscriber']) ? $data['Subscriptnote']['inscriber'] : array($data['Subscriptnote']['inscriber']);
        $node_wrapper->field_xk_inscriber->set($val);
        //题记时间
        if(isset($data['Subscriptnote']['modified'])) {
            $str = str_replace('[?]', '', $data['Subscriptnote']['modified']);
            $str = str_replace('[不详]', '', $data['Subscriptnote']['modified']);
            $str = str_replace('不详', '', $data['Subscriptnote']['modified']);
            $str = str_replace('日', '.', $data['Subscriptnote']['modified']);
            $str = str_replace('年', '.', $data['Subscriptnote']['modified']);
            $str = str_replace('月', '.', $data['Subscriptnote']['modified']);
            $str = str_replace('.', '-', $data['Subscriptnote']['modified']);
            $ta = explode('-', $str);
            $ta = array_map(function($item) {
                return trim($item);
            }, $ta);
            if(!isset($ta[1])) $ta[1] = '01';
            if(!isset($ta[2])) $ta[2] = '01';
            $str = implode('-', $ta);
            $str .= ' 00:00:00';
            if(strtotime($str)) {
                $node_wrapper->field_xk_modified->set(array(array('value' => $str, 'value2' => $str)));
            }
        }
        //题记类别
        if(isset($data['Subscriptnote']['typeOfInscriptions'])) {
            $tax = taxonomy_vocabulary_machine_name_load('tijileibie');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['Subscriptnote']['typeOfInscriptions']) ? $data['Subscriptnote']['typeOfInscriptions'] : array($data['Subscriptnote']['typeOfInscriptions']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
                 $node_wrapper->field_xk_typeofinscriptions->set($tt);
        }
        //题记书体
        if(isset($data['Subscriptnote']['calligraphyOfInscriptions'])) {
            $tax = taxonomy_vocabulary_machine_name_load('shuti');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['Subscriptnote']['calligraphyOfInscriptions']) ? $data['Subscriptnote']['calligraphyOfInscriptions'] : array($data['Subscriptnote']['calligraphyOfInscriptions']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
                 $node_wrapper->field_xk_calligraphyofinscriptio->set($tt);
        }
        //题记位置
        $val = is_array($data['Subscriptnote']['locationOfInscriptions']) ? $data['Subscriptnote']['locationOfInscriptions'] : array($data['Subscriptnote']['locationOfInscriptions']);
        $node_wrapper->field_xk_locationofinscriptions->set($val);
        //题记全文
        $val = is_array($data['Subscriptnote']['inscriptions']) ? $data['Subscriptnote']['inscriptions'] : array($data['Subscriptnote']['inscriptions']);
        $node_wrapper->field_xk_inscriptions->set($val);

    }
    //个人上款名
    if(isset($data['contributorPersonal'])) {
        $val = is_array($data['contributorPersonal']) ? $data['contributorPersonal'] : array($data['contributorPersonal']);
        $node_wrapper->field_dc_contributor_ming->set($val);
    }
    //团体上款名
    if(isset($data['contributorCorporate'])) {
        $val = is_array($data['contributorCorporate']) ? $data['contributorCorporate'] : array($data['contributorCorporate']);
        $node_wrapper->field_dc_contributor_ming1->set($val);
    }
    //上款内容
    if(isset($data['superscript'])) {
        $val = is_array($data['superscript']) ? $data['superscript'] : array($data['superscript']);
        $node_wrapper->field_superscript->set($val);
    }
     // 获取方式
     if(isset($data['availability'])) {
        $tax = taxonomy_vocabulary_machine_name_load('huoqufangshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['availability']) ? $data['availability'] : array($data['availability']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_obtainedby->set($tt);
        }
    }
    // 文献类型
    if(isset($data['type'])) {
        $tax = taxonomy_vocabulary_machine_name_load('_wenxinaleix');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['type']) ? $data['type'] : array($data['type']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_ceshi->set($tt);
        }
    }
    //关键词
    if(isset($data['keywords'])) {
        $val = is_array($data['keywords']) ? $data['keywords'] : array($data['keywords']);
        $node_wrapper->field_dc_subject->set($val);
    }
    //落款时间
    if(isset($data['created'])) {
        $val = is_array($data['created']) ? $data['created'] : array($data['created']);
        $node_wrapper->field_created->set($val);
    }
    //创作地
    if(isset($data['locationOfCreation'])) {
        $val = is_array($data['locationOfCreation']) ? $data['locationOfCreation'] : array($data['locationOfCreation']);
        $node_wrapper->field__di_useapi->set($val);
    }
    //数量及单位
    if(isset($data['amount'])) {
        $val = is_array($data['amount']) ? $data['amount'] : array($data['amount']);
        $node_wrapper->field_bf_extent->set($val);
    }
   //题记
   if(isset($data["TypeOfInscriptionsGroup"]))
   {
        //印章类型
        if(isset($data['TypeOfInscriptionsGroup']['typeOfSeal'])) {
            $tax = taxonomy_vocabulary_machine_name_load('yinzhangleixing');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['TypeOfInscriptionsGroup']['typeOfSeal']) ? $data['TypeOfInscriptionsGroup']['typeOfSeal'] : array($data['TypeOfInscriptionsGroup']['typeOfSeal']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
                 $node_wrapper->field_tj_typeofseal->set($tt);
        }
        //印章位置
        $val = is_array($data['TypeOfInscriptionsGroup']['locationOfSeal']) ? $data['TypeOfInscriptionsGroup']['locationOfSeal'] : array($data['TypeOfInscriptionsGroup']['locationOfSeal']);
        $node_wrapper->field_tj_locationofseal->set($val);
        //印章印文
        $val = is_array($data['TypeOfInscriptionsGroup']['inscriptionsOfSeal']) ? $data['TypeOfInscriptionsGroup']['inscriptionsOfSeal'] : array($data['TypeOfInscriptionsGroup']['inscriptionsOfSeal']);
        $node_wrapper->field_tj_inscriptionsofsea->set($val);
        //印文类型
        if(isset($data['TypeOfInscriptionsGroup']['typeOfSealInscriptions'])) {
            $tax = taxonomy_vocabulary_machine_name_load('yinwenleixing');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['TypeOfInscriptionsGroup']['typeOfSealInscriptions']) ? $data['TypeOfInscriptionsGroup']['typeOfSealInscriptions'] : array($data['TypeOfInscriptionsGroup']['typeOfSealInscriptions']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
                 $node_wrapper->field_tj_typeofsealinscriptions->set($tt);
        }
        //题记者
        $val = is_array($data['TypeOfInscriptionsGroup']['inscriber']) ? $data['TypeOfInscriptionsGroup']['inscriber'] : array($data['TypeOfInscriptionsGroup']['inscriber']);
        $node_wrapper->field_bf_agent2->set($val);
        //题记时间
        if(isset($data['TypeOfInscriptionsGroup']['modified'])) {
            $str = str_replace('[?]', '', $data['TypeOfInscriptionsGroup']['modified']);
            $str = str_replace('[不详]', '', $data['TypeOfInscriptionsGroup']['modified']);
            $str = str_replace('不详', '', $data['TypeOfInscriptionsGroup']['modified']);
            $str = str_replace('日', '.', $data['TypeOfInscriptionsGroup']['modified']);
            $str = str_replace('年', '.', $data['TypeOfInscriptionsGroup']['modified']);
            $str = str_replace('月', '.', $data['TypeOfInscriptionsGroup']['modified']);
            $str = str_replace('.', '-', $data['TypeOfInscriptionsGroup']['modified']);
            $ta = explode('-', $str);
            $ta = array_map(function($item) {
                return trim($item);
            }, $ta);
            if(!isset($ta[1])) $ta[1] = '01';
            if(!isset($ta[2])) $ta[2] = '01';
            $str = implode('-', $ta);
            $str .= ' 00:00:00';
            if(strtotime($str)) {
                $node_wrapper->field_dc_date->set(array(array('value' => $str, 'value2' => $str)));
            }
        }
        //题记类别
        if(isset($data['TypeOfInscriptionsGroup']['typeOfInscriptions'])) {
            $tax = taxonomy_vocabulary_machine_name_load('tijileibie');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['TypeOfInscriptionsGroup']['typeOfInscriptions']) ? $data['TypeOfInscriptionsGroup']['typeOfInscriptions'] : array($data['TypeOfInscriptionsGroup']['typeOfInscriptions']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
                 $node_wrapper->field_typeofinscription->set($tt);
        }
        //题记书体
        if(isset($data['TypeOfInscriptionsGroup']['calligraphyOfInscriptions'])) {
            $tax = taxonomy_vocabulary_machine_name_load('shuti');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['TypeOfInscriptionsGroup']['calligraphyOfInscriptions']) ? $data['TypeOfInscriptionsGroup']['calligraphyOfInscriptions'] : array($data['TypeOfInscriptionsGroup']['calligraphyOfInscriptions']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($item!='')
                    {
                        if($term->name == mb_substr($item,0,1,'utf-8')) {
                            array_push($tt,taxonomy_term_load($term->tid)) ;
                        }
                    }
                }
            }
            if(!empty($tt))
                 $node_wrapper->field_tijshuti->set($tt);
        }
        //题记位置
        $val = is_array($data['TypeOfInscriptionsGroup']['locationOfInscriptions']) ? $data['TypeOfInscriptionsGroup']['locationOfInscriptions'] : array($data['TypeOfInscriptionsGroup']['locationOfInscriptions']);
        $node_wrapper->field_shl_locationofinscription->set($val);
        //题记全文
        $val = is_array($data['TypeOfInscriptionsGroup']['inscriptions']) ? $data['TypeOfInscriptionsGroup']['inscriptions'] : array($data['TypeOfInscriptionsGroup']['inscriptions']);
        $node_wrapper->field_shl_contentofinscription->set($val);
   }

    //尺寸位置
    if(isset($data['SizeandLocation'])) {
        $val = is_array($data['SizeandLocation']['size']) ? $data['SizeandLocation']['size'] : array($data['SizeandLocation']['size']);
        $node_wrapper->field_bf_dimensions->set($val);
        $val = is_array($data['SizeandLocation']['location']) ? $data['SizeandLocation']['location'] : array($data['SizeandLocation']['location']);
        $node_wrapper->field_bf_location->set($val);
    }

     // 材质
     if(isset($data['MaterialsLocation'])) {
        //位置
        $val = is_array($data['MaterialsLocation']['location']) ? $data['MaterialsLocation']['location'] : array($data['MaterialsLocation']['location']);
        $node_wrapper->field_cz_location->set($val);
        //材质
        if(isset($data['MaterialsLocation']['materials'])) {
            $tax = taxonomy_vocabulary_machine_name_load('caizhi');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['MaterialsLocation']['materials']) ? $data['MaterialsLocation']['materials'] : array($data['MaterialsLocation']['materials']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
            {
                 $node_wrapper->field_shl_material->set($tt);
            }
        }

     }
     //创作工具
     if(isset($data['implement'])) {
        $tax = taxonomy_vocabulary_machine_name_load('shuxiegongju');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['implement']) ? $data['implement'] : array($data['implement']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_writingtool->set($tt);
        }
    }
    //技法
    if(isset($data['facture'])) {
        $val = is_array($data['facture']) ? $data['facture'] : array($data['facture']);
        $node_wrapper->field_shl_factureofcalligraphy->set($val);
    }
    //颜色
    if(isset($data['color'])) {
        $val = is_array($data['color']) ? $data['color'] : array($data['color']);
        $node_wrapper->field_shl_color->set($val);
    }
    //书体
    if(isset($data['styleOfCalligraphy'])) {
        $tax = taxonomy_vocabulary_machine_name_load('shuti');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['styleOfCalligraphy']) ? $data['styleOfCalligraphy'] : array($data['styleOfCalligraphy']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if(!empty($item))
                {
                    if($term->name == mb_substr($item,0,1,'utf-8')) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }

            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_fontofcalligraphy->set($tt);
        }
    }
    //闲章
    if(isset($data['Xianzhang'])) {

        //闲章类型
        if(isset($data['Xianzhang']['typeofxianzhang'])) {
            $tax = taxonomy_vocabulary_machine_name_load('xianzhangleibie');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['Xianzhang']['typeofxianzhang']) ? $data['Xianzhang']['typeofxianzhang'] : array($data['Xianzhang']['typeofxianzhang']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
            {
                 $node_wrapper->field_shl_typeofseal1->set($tt);
            }
        }
        //闲章位置
        if(isset($data['Xianzhang']['locationOfSeal'])) {
            $val = is_array($data['Xianzhang']['locationOfSeal']) ? $data['Xianzhang']['locationOfSeal'] : array($data['Xianzhang']['locationOfSeal']);
            $node_wrapper->field_xz_locationofseal->set($val);
        }
        //印章印文
        if(isset($data['Xianzhang']['inscriptionsOfSeal'])) {
            $val = is_array($data['Xianzhang']['inscriptionsOfSeal']) ? $data['Xianzhang']['inscriptionsOfSeal'] : array($data['Xianzhang']['inscriptionsOfSeal']);
            $node_wrapper->field_shl_inscriptionsofseal1->set($val);
        }
        //印文类型
        if(isset($data['Xianzhang']['typeOfSealInscriptions'])) {
            $tax = taxonomy_vocabulary_machine_name_load('typeOfSealInscriptions');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['Xianzhang']['typeOfSealInscriptions']) ? $data['Xianzhang']['typeOfSealInscriptions'] : array($data['Xianzhang']['typeOfSealInscriptions']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
            {
                 $node_wrapper->field_shl_typeofinscription1->set($tt);
            }
        }
    }
    //收传印章
    if(isset($data['Shouchuanseal'])) {
         //印章所有者
         if(isset($data['Shouchuanseal']['sealer'])) {
            $val = is_array($data['Shouchuanseal']['sealer']) ? $data['Shouchuanseal']['sealer'] : array($data['Shouchuanseal']['sealer']);
            $node_wrapper->field_shl_ownerofseal->set($val);
        }
         //闲章类型
         if(isset($data['Shouchuanseal']['typeofxianzhang'])) {
            $tax = taxonomy_vocabulary_machine_name_load('xianzhangleibie');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['Shouchuanseal']['typeofxianzhang']) ? $data['Shouchuanseal']['typeofxianzhang'] : array($data['Shouchuanseal']['typeofxianzhang']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
            {
                 $node_wrapper->field_shl_typeofseal->set($tt);
            }
        }
        //闲章位置
        if(isset($data['Shouchuanseal']['locationOfSeal'])) {
            $val = is_array($data['Shouchuanseal']['locationOfSeal']) ? $data['Shouchuanseal']['locationOfSeal'] : array($data['Shouchuanseal']['locationOfSeal']);
            $node_wrapper->field_scyz_locationofseal->set($val);
        }
        //印章印文
        if(isset($data['Shouchuanseal']['inscriptionsOfSeal'])) {
            $val = is_array($data['Shouchuanseal']['inscriptionsOfSeal']) ? $data['Shouchuanseal']['inscriptionsOfSeal'] : array($data['Shouchuanseal']['inscriptionsOfSeal']);
            $node_wrapper->field_shl_inscriptionsofseal->set($val);
        }
        //印文类型
        if(isset($data['Shouchuanseal']['typeOfSealInscriptions'])) {
            $tax = taxonomy_vocabulary_machine_name_load('yinwenleixing');
            $tree = taxonomy_get_tree($tax->vid);
            $tt = array();
            $val = is_array($data['Shouchuanseal']['typeOfSealInscriptions']) ? $data['Shouchuanseal']['typeOfSealInscriptions'] : array($data['Shouchuanseal']['typeOfSealInscriptions']);
            foreach($tree as $term) {
                foreach ($val as $item) {
                    if($term->name == $item) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }
            }
            if(!empty($tt))
            {
                 $node_wrapper->field_shl_typeofinscription->set($tt);
            }
        }
    }
    //一般性附注
    if(isset($data['dc_description'])) {
        $val = is_array($data['dc_description']) ? $data['dc_description'] : array($data['dc_description']);
        $node_wrapper->field_shl_description1->set($val);
    }
    //内容提要
    if(isset($data['abstracts_xml'])) {
        $val = is_array($data['abstracts_xml']) ? $data['abstracts_xml'] : array($data['abstracts_xml']);
        $node_wrapper->field_dct_abstract->set($val);
    }
    //行款
    if(isset($data['numberOfRows'])) {
        $val = is_array($data['numberOfRows']) ? $data['numberOfRows'] : array($data['numberOfRows']);
        $node_wrapper->field_shl_rowsandchars->set($val);
    }
    //装裱形式
    if(isset($data['mount'])) {
        $tax = taxonomy_vocabulary_machine_name_load('zhuangbiaoxingshi');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['mount']) ? $data['mount'] : array($data['mount']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($item!='')
                {
                    if($term->name ==  substr($item, 0, 1 )) {
                        array_push($tt,taxonomy_term_load($term->tid)) ;
                    }
                }

            }
        }
        if(!empty($tt))
        {
             $node_wrapper->field_shl_binding->set($tt);
        }
    }
    //载体形态附注
    if(isset($data['notesPertainingToPhysicalDescription'])) {
        $tax = taxonomy_vocabulary_machine_name_load('zaitixingtaifuzhu');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = array();
        $val = is_array($data['notesPertainingToPhysicalDescription']) ? $data['notesPertainingToPhysicalDescription'] : array($data['notesPertainingToPhysicalDescription']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_ccyz_ztxftz->set($tt);
    }
    //语种
    if(!isset($data['language']) || $data['language'] == '中文') $data['language'] = '汉语';
    if(isset($data['language'])) {
        $tax = taxonomy_vocabulary_machine_name_load('yuzhng');
        $tree = taxonomy_get_tree($tax->vid);

        $tt = array();
        $val = is_array($data['language']) ? $data['language'] : array($data['language']);
        foreach($tree as $term) {
            foreach ($val as $item) {
                if($term->name == $item) {
                    array_push($tt,taxonomy_term_load($term->tid)) ;
                }
            }
        }
        if(!empty($tt))
            $node_wrapper->field_dc_language->set($tt);
    }

    //个人捐赠者
    if(isset($data['personalDonator'])) {
        $val = is_array($data['personalDonator']) ? $data['personalDonator'] : array($data['personalDonator']);
        $node_wrapper->field_shl_donator->set($val);
    }
    //团体捐赠者
    if(isset($data['corporateDonator'])) {
        $val = is_array($data['corporateDonator']) ? $data['corporateDonator'] : array($data['corporateDonator']);
        $node_wrapper->field_shl_donator1->set($val);
    }
    //捐赠时间
    if(isset($data['donated'])) {
        $str = str_replace('[?]', '', $data['donated']);
        $str = str_replace('[不详]', '', $data['donated']);
        $str = str_replace('不详', '', $data['donated']);
        $str = str_replace('日', '.', $data['donated']);
        $str = str_replace('年', '.', $data['donated']);
        $str = str_replace('月', '.', $data['donated']);
        $str = str_replace('.', '-', $data['donated']);
        $ta = explode('-', $str);
        $ta = array_map(function($item) {
            return trim($item);
        }, $ta);
        if(!isset($ta[1])) $ta[1] = '01';
        if(!isset($ta[2])) $ta[2] = '01';
        $str = implode('-', $ta);
        $str .= ' 00:00:00';
        if(strtotime($str)) {
            $node_wrapper->field_dc_date2->set(array(array('value' => $str, 'value2' => $str)));
        }
    }
   //权限
    if(isset($data['Rights'])) {
        $val = is_array($data['Rights']) ? $data['Rights'] : array($data['Rights']);
        $node_wrapper->field_dc_rights->set($val);
    }

    // 密级
    if(isset($data['secret'])) {
        $tax = taxonomy_vocabulary_machine_name_load('miji');
        $tree = taxonomy_get_tree($tax->vid);
        $tt = NULL;
        foreach($tree as $term) {
            if($term->name == $data['secret']) {
                $tt = taxonomy_term_load($term->tid);
            }
        }
        if(!empty($tt))
            $node_wrapper->field_mi_ji->set($tt);
    }

    //相关资源
    if(isset($data['RelatedResource'])) {
        $val = is_array($data['RelatedResource']['reference']) ? $data['RelatedResource']['reference'] : array($data['RelatedResource']['reference']);
        $node_wrapper->field_reference->set($val);
        $val = is_array($data['RelatedResource']['callNoOfRelatedResource']) ? $data['RelatedResource']['callNoOfRelatedResource'] : array($data['RelatedResource']['callNoOfRelatedResource']);
        $node_wrapper->field_callnoofrelatedresource->set($val);
    }
    if(isset($data['selfUri'])) {
        $node_wrapper->field_selfurl->set($data['selfUri']);
    }
    $node_wrapper->field_data_source->set('olds');

    $node_wrapper->save();

    drupal_json_output(array('status' => 'success'));
    return;
}

function _custom_admin_zhengjiliushui(){
    $arg=drupal_get_query_parameters();
    
    if(isset($arg['term']) && strlen($arg['term'])>=2) {
            $or = db_or();
            $or->condition('l.field_liushuihao_value','%'.db_like($arg['term']) . '%', 'LIKE');
            $or->condition('n.title','%'.db_like($arg['term']) . '%', 'LIKE');

            $query=  db_select('field_data_field_liushuihao','l');
            $query->join('node','n',"n.nid=l.entity_id");
            $query->join('field_data_field_donated','f',"n.nid=f.entity_id");
            $result=$query->fields('l')
            ->fields('n')
            ->fields('f')
            ->condition($or)
            ->range(0, 10)
            ->execute();
            foreach ($result as $liushuihao) {

                $node= node_load($liushuihao->nid);

                $matches[] = (object)['id'=>$liushuihao->field_liushuihao_value,
                    'node'=>$node,
                    "label"=> $liushuihao->title . '(' . $liushuihao->field_liushuihao_value. ')-(' . substr($liushuihao->field_donated_value,0,10) .')',
                    "value"=> $liushuihao->title . '(' . $liushuihao->field_liushuihao_value. ')-(' . substr($liushuihao->field_donated_value,0,10) .')'

                ];
            }
    drupal_json_output($matches);
    }


}

function _custom_admin_zhengjiliushui_full(){
    $arg=drupal_get_query_parameters();
    
    if(isset($arg['term']) && strlen($arg['term'])>=2) {
            $or = db_or();
            $or->condition('l.field_liushuihao_value',$arg['term'], '=');
            $or->condition('n.title',$arg['term'], '=');

            $query=  db_select('field_data_field_liushuihao','l');
            $query->join('node','n',"n.nid=l.entity_id");
            $query->join('field_data_field_donated','f',"n.nid=f.entity_id");
            $result=$query->fields('l')
            ->fields('n')
            ->fields('f')      
            ->condition($or)
            ->range(0, 10)
            ->execute();
            foreach ($result as $liushuihao) {

                $node= node_load($liushuihao->nid);

                $matches[] = (object)['id'=>$liushuihao->field_liushuihao_value,
                    'node'=>$node,
                    "label"=> $liushuihao->title . '(' . $liushuihao->field_liushuihao_value. ')-(' . substr($liushuihao->field_donated_value,0,10) .')',
                    "value"=> $liushuihao->title . '(' . $liushuihao->field_liushuihao_value. ')-(' . substr($liushuihao->field_donated_value,0,10) .')'

                ];
            }
    drupal_json_output($matches);
    }
}
