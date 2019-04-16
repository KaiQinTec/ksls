<div class="insert-analysis col-md-12 col-xs-12"></div>
<div class="all-insert-analysis col-md-12 col-xs-12"></div>
<style type="text/css">
    .insert-analysis,
    .all-insert-analysis {
        height: 600px;
    }
</style>
<script type="text/javascript">
(function($) {
    var date = new Date();

    option = {
        title : {
            text: '本月著录统计',
            subtext: date.getFullYear() + '年' + (date.getMonth() + 1) + '月',
            x:'center'
        },
        tooltip: {
            trigger: 'item',
            formatter: "{a} <br/>{b}: {c} ({d}%)"
        },
        legend: {
            orient: 'vertical',
            x: 'left',
            data:[<?php foreach($current_month_count_list as $item) { print '"' . $item['name'] . '",'; }?>]
        },
        series: [
            {
                name:'著录统计',
                type:'pie',
                selectedMode: 'single',
                radius: [0, '30%'],

                label: {
                    normal: {
                        position: 'inner'
                    }
                },
                labelLine: {
                    normal: {
                        show: false
                    }
                },
                data:[
                    <?php
                    foreach($current_month_count_list as $key => $item) {
                        /*if($item['count'] <= 0) {
                            $item['count'] = rand(50, 100);
                            $current_month_count_list[$key]['count'] = $item['count'];
                        }*/
                        print '{value:' . $item['count'] . ', name:"' . $item['name'] . '"},';
                    }
                    ?>
                ]
            },
            {
                name:'著录统计',
                type:'pie',
                radius: ['40%', '55%'],
                label: {
                    normal: {
                        formatter: '{a|{a}}{abg|}\n{hr|}\n  {b|{b}：}{c}  {per|{d}%}  ',
                        backgroundColor: '#eee',
                        borderColor: '#aaa',
                        borderWidth: 1,
                        borderRadius: 4,
                        rich: {
                            a: {
                                color: '#999',
                                lineHeight: 22,
                                align: 'center'
                            },
                            hr: {
                                borderColor: '#aaa',
                                width: '100%',
                                borderWidth: 0.5,
                                height: 0
                            },
                            b: {
                                fontSize: 16,
                                lineHeight: 33
                            },
                            per: {
                                color: '#eee',
                                backgroundColor: '#334455',
                                padding: [2, 4],
                                borderRadius: 2
                            }
                        }
                    }
                },
                data:[
                    <?php
                    foreach($current_month_count_list as $item) {
                        print '{value:' . $item['count'] . ', name:"' . $item['name'] . '"},';
                    }
                    ?>
                ]
            }
        ]
    };

    var myChart = echarts.init($('.insert-analysis')[0]);
    myChart.setOption(option);

    option.title.text = '全部数据统计';
    option.title.subtext = '';
    option.legend.data = [<?php foreach($count_list as $item) { print '"' . $item['name'] . '",'; }?>];
    option.series[0].data = option.series[1].data = [
        <?php
        foreach($count_list as $key => $item) {
            /*if($item['count'] <= 0) {
                $item['count'] = rand(50, 100);
                $count_list[$key]['count'] = $item['count'];
            }*/
            print '{value:' . $item['count'] . ', name:"' . $item['name'] . '"},';
        }
        ?>
    ];
    var allChart = echarts.init($('.all-insert-analysis')[0]);
    allChart.setOption(option);
})(jQuery)

</script>