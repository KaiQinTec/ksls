<?php

/**
 * @file
 * Default theme implementation to display a single Drupal page.
 *
 * The doctype, html, head and body tags are not in this template. Instead they
 * can be found in the html.tpl.php template in this directory.
 *
 * Available variables:
 *
 * General utility variables:
 * - $base_path: The base URL path of the Drupal installation. At the very
 *   least, this will always default to /.
 * - $directory: The directory the template is located in, e.g. modules/system
 *   or themes/bartik.
 * - $is_front: TRUE if the current page is the front page.
 * - $logged_in: TRUE if the user is registered and signed in.
 * - $is_admin: TRUE if the user has permission to access administration pages.
 *
 * Site identity:
 * - $front_page: The URL of the front page. Use this instead of $base_path,
 *   when linking to the front page. This includes the language domain or
 *   prefix.
 * - $logo: The path to the logo image, as defined in theme configuration.
 * - $site_name: The name of the site, empty when display has been disabled
 *   in theme settings.
 * - $site_slogan: The slogan of the site, empty when display has been disabled
 *   in theme settings.
 *
 * Navigation:
 * - $main_menu (array): An array containing the Main menu links for the
 *   site, if they have been configured.
 * - $secondary_menu (array): An array containing the Secondary menu links for
 *   the site, if they have been configured.
 * - $breadcrumb: The breadcrumb trail for the current page.
 *
 * Page content (in order of occurrence in the default page.tpl.php):
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title: The page title, for use in the actual HTML content.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 * - $messages: HTML for status and error messages. Should be displayed
 *   prominently.
 * - $tabs (array): Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node).
 * - $action_links (array): Actions local to the page, such as 'Add menu' on the
 *   menu administration interface.
 * - $feed_icons: A string of all feed icons for the current page.
 * - $node: The node object, if there is an automatically-loaded node
 *   associated with the page, and the node ID is the second argument
 *   in the page's path (e.g. node/12345 and node/12345/revisions, but not
 *   comment/reply/12345).
 *
 * Regions:
 * - $page['help']: Dynamic help text, mostly for admin pages.
 * - $page['highlighted']: Items for the highlighted content region.
 * - $page['content']: The main content of the current page.
 * - $page['sidebar_first']: Items for the first sidebar.
 * - $page['sidebar_second']: Items for the second sidebar.
 * - $page['header']: Items for the header region.
 * - $page['footer']: Items for the footer region.
 *
 * @see template_preprocess()
 * @see template_preprocess_page()
 * @see template_process()
 * @see html.tpl.php
 *
 * @ingroup themeable
 */
?>
<style type="text/css">
.table-bordered,
.table-bordered tr {
    background-color: #fff;
    font-size: 14px;
}
.table a {
    color: #337ab7 !important;
    text-decoration: underline;
}
</style>
<div class="navbar navbar-default" id="navbar">
    <script type="text/javascript">
        try {
            ace.settings.check('navbar', 'fixed')
        } catch (e) {
        }
    </script>
    <div class="navbar-container" id="navbar-container" style="height: 80px">
        <div class="navbar-header pull-left">
            <a href="#" class="navbar-brand">
                <small>
                    <img src="/<?php echo $path; ?>/images/logo01.png" width="470px">
                </small>
            </a><!-- /.brand -->
        </div><!-- /.navbar-header -->
        <div class="navbar-header operating pull-left">

        </div>
        <div class="navbar-header pull-right" role="navigation">
            <ul class="nav ace-nav">
                <li class="light-blue">
                    <a data-toggle="dropdown" href="#" class="dropdown-toggle">
                        <span class="time"><em id="time"></em></span><span class="user-info"><small>欢迎光临,</small><?php echo $user->name; ?></span>
                        <i class="icon-caret-down"></i>
                    </a>
                    <ul class="user-menu pull-right dropdown-menu dropdown-yellow dropdown-caret dropdown-close">
                        <!--<li><a href="javascript:void(0" name="Systems.html" title="系统设置" class="iframeurl"><i class="icon-cog"></i>网站设置</a></li>-->
                        <!-- <li><a href="javascript:void(0)" name="admin_info.html" title="个人信息" class="iframeurl"><i
                                class="/user"></i>个人资料</a></li>
                        <li class="divider"></li> -->
                        <li><a href="/user/logout" id="Exit_system"><i class="icon-off"></i>退出</a></li>
                    </ul>
                </li>
            </ul>
        </div>
    </div>
</div>
<div class="main-container" id="main-container">
    <div class="main-container-inner">
        <a class="menu-toggler" id="menu-toggler" href="#">
            <span class="menu-text"></span>
        </a>
        <div class="sidebar" id="sidebar">
            <div class="sidebar-shortcuts" id="sidebar-shortcuts">
                <div class="sidebar-shortcuts-large" id="sidebar-shortcuts-large">
                    <a href="<?php echo url('/search/gaoji'); ?>" title="高级检索" class="btn btn-warning">
                        <i class="icon-search"></i>
                    </a>
                    <!-- <a href="<?php echo url('user-list/donation'); ?>" title="捐献人管理" class="btn btn-success">
                        <i class="icon-file"></i>
                    </a>
                    <a href="<?php echo url('user-list/creator'); ?>" title="责任者管理" class="btn btn-warning">
                        <i class="icon-group"></i>
                    </a> -->
                </div>

                <div class="sidebar-shortcuts-mini" id="sidebar-shortcuts-mini">
                    <span class="btn btn-success"></span>
                    <span class="btn btn-info"></span>
                    <span class="btn btn-warning"></span>
                    <span class="btn btn-danger"></span>
                </div>
            </div><!-- #sidebar-shortcuts -->
            <div id="menu_style" class="menu_style">
                <ul class="nav nav-list" id="nav_list">
                    <li class="home">
                        <a href="<?php echo url('front'); ?>" title="">
                            <i class="icon-home"></i><span class="menu-text"> 系统首页 </span>
                        </a>
                    </li>
                    <li>
                        <a href="#" class="dropdown-toggle">
                            <i class="icon-edit"></i><span class="menu-text"> 著录管理 </span><b class="arrow icon-check"></b>
                        </a>
                        <ul class="submenu">
                            <?php //foreach($content_types as $type => $name) :?>
                            <?php //$type = str_replace('_', '-', $type);?>
                            <li class="home">
                                <a href="<?php echo url('content/zhengji'); ?>">
                                    <i class="icon-double-angle-right"></i>内容管理
                                </a>
                            </li>
                            <?php if (user_has_role(3, $user) || user_has_role(5, $user) || user_has_role(7, $user)): ?>
                                <li class="home">
                                    <a href="<?php echo url('content/content-list'); ?>">
                                        <i class="icon-double-angle-right"></i>审核
                                    </a>
                                </li>
                            <?php endif; ?>
                            <li class="home">
                                <a href="javascript:void(0);" title="" id="add-new-node">
                                    <i class="icon-double-angle-right"></i>著录
                                </a>
                            </li>
                            <?php //endforeach;?>

                        </ul>
                    </li>
                    <li>
                        <a href="#" class="dropdown-toggle"><i class="icon-bar-chart"></i><span class="menu-text"> 统计分析 </span><b
                                class="arrow icon-angle-down"></b></a>
                        <ul class="submenu">
                            <li class="home">
                                <a href="/insert/count" name="Cover_management.html" title="著录统计" class="iframeurl">
                                    <i class="icon-double-angle-right"></i>著录统计
                                </a>
                            </li>
                            <li class="home">
                                <a href="/insert/count/zhengji" name="payment_method.html" title="征集著录统计" class="iframeurl">
                                    <i class="icon-double-angle-right"></i>征集著录统计
                                </a>
                            </li>
                            <li class="home">
                                <a href="/insert/count/shenhe" name="payment_method.html" title="审核统计" class="iframeurl">
                                    <i class="icon-double-angle-right"></i>审核统计
                                </a>
                            </li>
                            <li class="home">
                                <a href="/insert/count/juanxian" name="Payment_Configure.html" title="捐献统计" class="iframeurl">
                                    <i class="icon-double-angle-right"></i>捐献统计
                                </a>
                            </li>
                            <li class="home">
                                <a href="/analysis/all" name="Payment_Configure.html" title="可视化统计" class="iframeurl">
                                    <i class="icon-double-angle-right"></i>可视化统计
                                </a>
                            </li>
                        </ul>
                    </li>
                    <?php if (user_access('administer content types', $user)): ?>
                    <li>
                        <a href="#" class="dropdown-toggle">
                            <i class="icon-list "></i><span class="menu-text"> 结构管理 </span><b class="arrow icon-angle-down"></b>
                        </a>
                        <ul class="submenu">
                            <?php foreach ($structure_menu as $menu): ?>
                            <li class="home">
                                <a href="<?php echo url($menu['link_path']); ?>" title="<?php echo $menu['link_title']; ?>">
                                    <i class="icon-double-angle-right"></i><?php echo $menu['link_title']; ?>
                                </a>
                            </li>
                            <?php endforeach; ?>
                        </ul>
                    </li>
                    <?php endif; ?>
                    <?php if (user_access('administer users', $user)): ?>
                    <li>
                        <a href="#" class="dropdown-toggle"><i class="icon-user"></i><span
                                class="menu-text"> 用户权限管理 </span><b class="arrow icon-angle-down"></b></a>
                        <ul class="submenu">
                            <li class="home">
                                <a href="/admin/people" name="user_list.html" title="用户管理" class="iframeurl">
                                    <i class="icon-double-angle-right"></i>用户管理
                                </a>
                            </li>
                            <li class="home">
                                <a href="/admin/people/permissions/roles" name="member-Grading.html" title="角色管理" class="iframeurl">
                                    <i class="icon-double-angle-right"></i>角色管理
                                </a>
                            </li>
                            <li class="home">
                                <a href="/admin/people/permissions/roles" name="Shop_list.html" title="权限管理" class="iframeurl">
                                    <i class="icon-double-angle-right"></i>权限管理
                                </a>
                            </li>

                        </ul>
                    </li>
                    <?php endif; ?>
                    <?php if (user_access('administer users', $user)): ?>
                    <li>
                            <a href="/admin" >
                                <i class="icon-cog"></i>
                                <span class="menu-text"> 系统管理 </span>
                            </a>
                    </li>
                    <?php endif; ?>
                </ul>
            </div>
            <script type="text/javascript">
                jQuery("#menu_style").niceScroll({
                    cursorcolor: "#888888",
                    cursoropacitymax: 1,
                    touchbehavior: false,
                    cursorwidth: "5px",
                    cursorborder: "0",
                    cursorborderradius: "5px"
                });
            </script>
            <div class="sidebar-collapse" id="sidebar-collapse">
                <i class="icon-double-angle-left" data-icon1="icon-double-angle-left"
                   data-icon2="icon-double-angle-right"></i>
            </div>
            <script type="text/javascript">
                try {
                    ace.settings.check('sidebar', 'collapsed')
                } catch (e) {
                }
            </script>
        </div>

        <div class="main-content">
            <div class="breadcrumbs" id="breadcrumbs">
                <ul class="breadcrumb">
                    <li>
                        <i class="icon-home home-icon"></i>
                        <a href="<?php echo url('/'); ?>">首页</a>
                    </li>
                    <li class="active"><span class="Current_page iframeurl"><?php echo $title; ?></span></li>
                </ul>
            </div>
            <div class="search-result col-md-9">
                <?php echo render($page['content']); ?>
            </div>
            <div class="left-bar col-md-3">
                <?php if (isset($_GET['field_dc_creator'])): ?>
                <div id="currrent-search-key" class="block block-facetapi contextual-links-region">
                    <h2>当前搜索</h2>
                    <div class="content">
                        <div class="item-list">
                            <ul class="facetapi-facetapi-links facetapi-facet-type facetapi-processed">
                                <li class="leaf last"><?php echo $_GET['field_dc_creator']; ?></li>
                            </ul>
                            <?php echo $juanzeng_date; ?>
                        </div>
                    </div>
                </div>
                <?php endif; ?>
                <?php echo render($page['leftbar']); ?>
            </div>

            <!--iframe id="iframe" style="border:0; width:100%; background-color:#FFF;" name="iframe" frameborder="0"
                    src="home.html"></iframe-->


            <!-- /.page-content -->
        </div><!-- /.main-content -->

    </div><!-- /.main-container-inner -->

</div>
<!--底部样式-->

<div class="footer_style" id="footerstyle">
    <p class="l_f">版权所有：上海市历史博物馆</p>
</div>
<!--修改密码样式-->
<div class="change_Pass_style" id="change_Pass">
    <ul class="xg_style">
        <li><label class="label_name">原&nbsp;&nbsp;密&nbsp;码</label><input name="原密码" type="password" class=""
                                                                          id="password"></li>
        <li><label class="label_name">新&nbsp;&nbsp;密&nbsp;码</label><input name="新密码" type="password" class=""
                                                                          id="Nes_pas"></li>
        <li><label class="label_name">确认密码</label><input name="再次确认密码" type="password" class="" id="c_mew_pas"></li>
    </ul>
</div>
<!-- BEGIN　展示节点内容 Modal -->
<div class="modal fade" id="modalShowNode" tabindex="-1" role="dialog" aria-labelledby="modalShowNodeLabel">
    <div class="modal-dialog" role="document">
        <div class="modal-content">
            <div class="modal-header">
                <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                    <span aria-hidden="true">&times;</span>
                </button>
                <h4 class="modal-title" id="modalShowNodeTypesLabel">内容展示</h4>
            </div>
            <div class="modal-body ">
                内容加载中...
            </div>
            <div class="modal-footer">
                <button type="button" class="btn grey-mint" data-dismiss="modal">关闭</button>
            </div>
        </div>
    </div>
</div>
<!-- END　展示节点内容 Modal -->
<!-- /.main-container -->
<!-- basic scripts -->
<script type="text/javascript">
        (function ($) {
            var cid = $('#nav_list> li>.submenu');
            cid.each(function (i) {
                $(this).attr('id', "Sort_link_" + i);

            })
        })(jQuery)
        var $ = jQuery;
        jQuery(document).ready(function ($) {
            $.each($(".submenu"), function () {
                var $aobjs = $(this).children("li");
                var rowCount = $aobjs.size();
                var divHeigth = $(this).height();
                $aobjs.height(divHeigth / rowCount);
            });
            //初始化宽度、高度

            /*$("#main-container").height($(window).height() - 76);
            $("#iframe").height($(window).height() - 140);

            $(".sidebar").height($(window).height() - 99);
            var thisHeight = $("#nav_list").height($(window).outerHeight() - 173);
            $(".submenu").height();
            $("#nav_list").children(".submenu").css("height", thisHeight);*/

            //当文档窗口发生改变时 触发
            $(window).resize(function () {
                //$("#main-container").height($(window).height() - 76);
                $("#iframe").height($(window).height() - 140);
                $(".sidebar").height($(window).height() - 99);

                var thisHeight = $("#nav_list").height($(window).outerHeight() - 173);
                $(".submenu").height();
                $("#nav_list").children(".submenu").css("height", thisHeight);
            });

        });
        /******/
        $(document).on('click', '.link_cz > li', function () {
            $('.link_cz > li').removeClass('active');
            $(this).addClass('active');
        });

        /*********************点击事件*********************/
        $(document).ready(function () {
            $('#nav_list,.link_cz').find('li.home').on('click', function () {
                $('#nav_list,.link_cz').find('li.home').removeClass('active');
                $(this).addClass('active');
            });

//时间设置
            function currentTime() {
                var d = new Date(), str = '';
                str += d.getFullYear() + '年';
                str += d.getMonth() + 1 + '月';
                str += d.getDate() + '日';
                str += d.getHours() + '时';
                str += d.getMinutes() + '分';
                str += d.getSeconds() + '秒';
                return str;
            }

            setInterval(function () {
                $('#time').html(currentTime)
            }, 1000);

        });

    </script>