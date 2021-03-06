<div class="page-content clearfix">
    <div class="alert alert-block alert-success">
        <button type="button" class="close" data-dismiss="alert"><i class="icon-remove"></i></button>
        <i class="icon-ok green"></i>欢迎使用<strong class="green"><?php echo $site_name; ?></strong>,你本次登录时间为<?php echo $login_time; ?>，登录IP:<?php echo $ip_address; ?>.
    </div>
    <div class="state-overview clearfix">
        <div class="col-lg-3 col-sm-6">
            <section class="panel">
                <div class="symbol yellow">
                    <i class="icon-copy"></i>
                </div>
                <div class="value">
                    <h1><?php echo array_sum(array_column($needs_review, 'count')); ?></h1>
                    <p>待审核总数</p>
                </div>
            </section>
        </div>
        <div class="col-lg-3 col-sm-6">
            <section class="panel">
                    <div class="symbol terques">
                        <i class="icon-book"></i>
                    </div>
                    <div class="value">
                        <h1><?php echo array_sum(array_column($current_month_count_list, 'count')); ?></h1>
                        <p>当月著录总数</p>
                    </div>
            </section>
        </div>
        <div class="col-lg-3 col-sm-6">
            <section class="panel">
                <div class="symbol green">
                    <i class="icon-check"></i>
                </div>
                <div class="value">
                    <h1><?php echo array_sum(array_column($published, 'count')); ?></h1>
                    <p>当月审核总数</p>
                </div>
            </section>
        </div>
    </div>
    <div class="Order_Statistics">
        <div class="title_name"><?php if (user_has_role(4, $user)) {
    echo '审核未通过';
} else {
    echo '待审核信息';
}?></div>
        <table class="table table-bordered">
            <tbody>
            <?php foreach ($needs_review as $type_key => $type): ?>
                <tr>
                    <td class="name"><?php echo $type['name']; ?>：</td>
                    <td class="munber"><a href="/content/content-list<?php echo '?type=' . $type_key . '&state=needs_review'; ?>"><?php echo $type['count']; ?></a>&nbsp;件</td>
                </tr>
            <?php endforeach;?>

            <?php foreach ($no_published as $type_key => $type): ?>
                <tr>
                    <td class="name"><?php echo $type['name']; ?>：</td>
                    <td class="munber"><a href="/admin/content<?php echo '?type=' . $type_key . '&state=no_published'; ?>"><?php echo $type['count']; ?></a>&nbsp;件</td>
                </tr>
            <?php endforeach;?>
            </tbody>
        </table>
    </div>
    <!--实时交易记录-->
    <div class="clearfix">
        <div class="Order_Statistics ">
            <div class="title_name">当月著录统计信息</div>
            <table class="table table-bordered">
                <tbody>
                <?php foreach ($current_month_count_list as $type_key => $type): ?>
                <tr>
                    <td class="name"><?php echo $type['name']; ?>：</td>
                    <td class="munber">
                        <a href="<?php echo url('admin/content', ['query' => ['type' => $type_key, 'created' => $start_date, 'created_1' => $end_date]]); ?>">
                            <?php echo $type['count']; ?></a>&nbsp;件
                        </td>
                </tr>
                <?php endforeach;?>
                </tbody>
            </table>
        </div>
        <div class="Order_Statistics">
            <div class="title_name">当月审核统计信息</div>
            <table class="table table-bordered">
                <tbody>
                <?php foreach ($published as $type_key => $type): ?>
                <tr>
                    <td class="name"><?php echo $type['name']; ?>：</td>
                    <td class="munber">
                        <a href="<?php echo url('content/content-list', ['query' => ['type' => $type_key, 'state' => 'published', 'stamp[min]' => $start_date, 'stamp[max]' => $end_date]]); ?>">
                            <?php echo $type['count']; ?>
                        </a>&nbsp;件
                    </td>
                </tr>
                <?php endforeach;?>
                </tbody>
            </table>
        </div>


    </div>
    <!--记录-->
    <div class="clearfix">
        <div class="home_btn">
            <div>

                <a href="javascript:void(0);"  title="数据采集" id="add-new-node" class="btn  btn-info btn-sm no-radius">
                    <i class="bigger-200"><img src="<?php echo $theme_path; ?>/images/icon-addwz.png"/></i>
                    <h5 class="margin-top">数据采集</h5>
                </a>
                <a href="<?php echo url('admin/content'); ?>" title="数据管理" class="btn  btn-purple btn-sm no-radius">
                    <i class="bigger-200"><img src="<?php echo $theme_path; ?>/images/star-on-big.png"/></i>
                    <h5 class="margin-top">数据管理</h5>
                </a>
                <a href="<?php echo url('search/gaoji'); ?>" title="高级检索" class="btn  btn-info btn-sm no-radius">
                    <i class="bigger-200"><img src="<?php echo $theme_path; ?>/images/gaojisearch.png"/></i>
                    <h5 class="margin-top">高级检索</h5>
                </a>
                <!-- <a href="<?php echo url('insert/count/shenhe'); ?>" title="审核统计" class="btn  btn-success btn-sm no-radius">
                    <i class="bigger-200"><img src="<?php echo $theme_path; ?>/images/icon_right_s.png"/></i>
                    <h5 class="margin-top">审核统计</h5>
                </a> -->
                <a href="<?php echo url('analysis/all'); ?>" title="可视化统计" class="btn  btn-pink btn-sm no-radius">
                    <i class="bigger-200"><img src="<?php echo $theme_path; ?>/images/tj2.png"/></i>
                    <h5 class="margin-top">可视化统计</h5>
                </a>

            </div>
        </div>

    </div>

</div>

<script type="text/javascript">
(function($) {
    //面包屑返回值
    var index = parent.layer.getFrameIndex(window.name);
    parent.layer.iframeAuto(index);
    $('.no-radius').on('click', function () {
        var cname = $(this).attr("title");
        var chref = $(this).attr("href");
        var cnames = parent.$('.Current_page').html();
        var herf = parent.$("#iframe").attr("src");
        parent.$('#parentIframe').html(cname);
        parent.$('#iframe').attr("src", chref).ready();
        ;
        parent.$('#parentIframe').css("display", "inline-block");
        parent.$('.Current_page').attr({"name": herf, "href": "javascript:void(0)"}).css({
            "color": "#4c8fbd",
            "cursor": "pointer"
        });
        parent.layer.close(index);

    });
    $(document).ready(function () {

        $(".t_Record").width($(window).width() - 640);
        //当文档窗口发生改变时 触发
        $(window).resize(function () {
            $(".t_Record").width($(window).width() - 640);
        });
    });
})(jQuery);

</script>