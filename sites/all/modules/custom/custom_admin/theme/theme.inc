<?php

/**
 * custom-front-page.tpl.php
 */
function template_preprocess_custom_front_page(&$vars) {
    global $user;
    $vars['theme_path'] = drupal_get_path('theme', 'book_library');
    $vars['site_name'] = variable_get('site_name');
    $vars['ip_address'] = ip_address();
    $vars['login_time'] = date('Y年m月d日 H时i分', $user->login);

    // 获取著录角色用户总数
    $query = db_select('users_roles', 'roles')->fields('roles', array('uid'))->condition('rid', 4);
    $vars['insert_user_count'] = $query->execute()->rowCount();
    // 征集总数
    $query = db_select('node', 'node')->fields('node', array('nid', 'type'));
    $query->leftJoin('field_data_field_shzkzjzxm', 'zjz', 'zjz.entity_id = node.nid');
    $query->addExpression('COUNT(node.nid)', 'node_count');
    $query->isNotNull('zjz.field_shzkzjzxm_target_id');
    $query->where("FROM_UNIXTIME(node.created, '%Y-%m') = :date", array('date' => date('Y-m', REQUEST_TIME)));
    $result = $query->execute()->fetchCol(2);
    $vars['current_month_zhengji'] = current($result);

    // 获取所有节点类型
    $node_types = node_type_get_types();

    $query = db_select('node', 'node')->fields('node', array('nid', 'type'));
    $query->rightJoin('node_type', 'node_type', 'node.type = node_type.type');
    $query->addExpression('COUNT(node.nid)', 'node_count');
    $query->where("FROM_UNIXTIME(node.created, '%Y-%m') = :date", array('date' => date('Y-m', REQUEST_TIME)));
    
    $query->groupBy('node.type');
    //$string = $query->_toString();
    $result = $query->execute()->fetchAll();
    $counts = array();
    foreach($result as $v) {
        $counts[$v->type] = $v->node_count;
    }

    $current_month_count_list = array();
    $current_month_count_list = array_map(function($type) use($counts) {
        return array(
            'count' => isset($counts[$type->type]) ? $counts[$type->type] : 0,
            'name' => $type->name
        );
    }, $node_types);
    $vars['current_month_count_list'] = $current_month_count_list;


    //当月审核
    $query = db_select('node', 'node')->fields('node', array('nid', 'type'));
    $query->rightJoin('node_type', 'node_type', 'node.type = node_type.type');
    $query->addExpression('COUNT(node.nid)', 'node_count');
    $query->leftJoin('workbench_moderation_node_history', 'state', 'node.nid = state.nid');
    $query->condition('state.is_current', 1);
    $query->condition('state.state', 'needs_review');
    $query->where("FROM_UNIXTIME(state.stamp, '%Y-%m') = :date", array('date' => date('Y-m', REQUEST_TIME)));
    $result = $query->execute()->fetchAll();
    $counts = array();
    foreach($result as $v) {
        $counts[$v->type] = $v->node_count;
    }
    $current_month_shcount_list = array();
    $current_month_shcount_list = array_map(function($type) use($counts) {
        return array(
            'count' => isset($counts[$type->type]) ? $counts[$type->type] : 0,
            'name' => $type->name
        );
    }, $node_types);
    $vars['current_month_shcount_list'] = $current_month_shcount_list;


    $query = db_select('node', 'node')->fields('node', array('nid', 'type'));
    $query->rightJoin('node_type', 'node_type', 'node.type = node_type.type');
    $query->addExpression('COUNT(node.nid)', 'node_count');
    $query->leftJoin('workbench_moderation_node_history', 'state', 'node.nid = state.nid');
    $query->condition('state.is_current', 1);
    
    // 著录人员登录时，显示审核未通过的内容
    if(user_has_role(4, $user)) {
        $query->condition('state.state', 'no_published');
        $query->condition('node.uid', $user->uid);
    } else {
        $query->condition('state.state', 'needs_review');
    }
    //$query->where("FROM_UNIXTIME(node.created, '%Y-%m') = :date", array('date' => date('Y-m', REQUEST_TIME)));
    $query->groupBy('node.type');
    //$string = $query->__toString();
    $result = $query->execute()->fetchAll();

    $counts = array();
    foreach($result as $v) {
        $counts[$v->type] = $v->node_count;
    }

    $needs_review = array();
    $needs_review = array_map(function($type) use($counts) {
        return array(
            'count' => isset($counts[$type->type]) ? $counts[$type->type] : 0,
            'name' => $type->name
        );
    }, $node_types);

    $vars['no_published'] = array();
    $vars['needs_review'] = array();
    if(user_has_role(4, $user)) {
        $vars['no_published'] = $needs_review;
    } else {
        $vars['needs_review'] = $needs_review;
    }

    // 审核校验信息
    $query = db_select('node', 'node')->fields('node', array('nid', 'type'));
    $query->rightJoin('node_type', 'node_type', 'node.type = node_type.type');
    $query->addExpression('COUNT(node.nid)', 'node_count');
    $query->leftJoin('workbench_moderation_node_history', 'state', 'node.nid = state.nid');
    $query->condition('state.is_current', 1);
    $query->condition('state.state', 'published');
    $query->where("FROM_UNIXTIME(state.stamp, '%Y-%m') = :date", array('date' => date('Y-m', REQUEST_TIME)));
    $query->groupBy('node.type');
    $result = $query->execute()->fetchAll();
    //$string = $query->__toString();
    $counts = array();
    foreach($result as $v) {
        $counts[$v->type] = $v->node_count;
    }

    $published = array();
    $published = array_map(function($type) use($counts) {
        return array(
            'count' => isset($counts[$type->type]) ? $counts[$type->type] : 0,
            'name' => $type->name
        );
    }, $node_types);
    $vars['published'] = $published;

    $vars['start_date'] = date('m/01/Y', REQUEST_TIME);
    $vars['end_date'] = date('m/d/Y', strtotime($vars['start_date'] . " +1 month -1 day"));
}