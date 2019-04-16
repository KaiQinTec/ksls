(function($) {
    $(document).ready(function() {
        $('#import_next').on('click', function(e) {
            $.ajax({
                url: '/qianmingben.json',
                type: 'GET',
                dataType: 'JSON',
                success: function(data) {
                    do_next(data);
                }
            });
        });
    });

    function do_next(data) {
        var start = parseInt($('#import_next').attr('data-current'));
        //start = 1092;
        start = start == 0 ? 0 : start + 1;
        var end = start + 999;
        for(var index = start; index <= end; index ++) {
            if(index <= data.length) {
                $.ajax({
                    url: '/import/data/insert',
                    type: 'POST',
                    dataType: 'JSON',
                    data: data[index],
                    async: false,
                    success: function(response) {
                        //console.log(response);
                    }
                });
            }
        }
        $('h1').text('已导入数据：' + end);
        $('#import_next').attr('data-current', end);
    }
})(jQuery)