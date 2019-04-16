(function($) {
    $(document).ready(function() {

    	autocomplete_liushui();

        $('#edit-submit-admin-views-node').text('查询');
        $('#edit-submit-admin-views-node').val('查询');

        $('select[name="workbench_access"]').find('option').eq(1).attr('selected', true);

        // 点击著录，弹出类型选择
        $('#create_node, #add-new-node').bind('click', function() {
            $('#modalContentTypes').modal('show');

            var type = $.cookie('selected_type');
            if(type != undefined) {

                $('#select-type').find('option').each(function(i) {
                    if($(this).attr('value') == type) {
                        $(this).attr('selected', true);
                    }
                });
            }
        });
        // 点击选择，跳转到对应的添加内容页面
        $('.select-type').bind('click', function() {
            var type = $('#select-type').val();
            if(type == null) {
                alert('请选择要著录的类型');
                return;
            }

            $.cookie('selected_type', type);

            type = type.replace(/_/g, '-');

            var path = Drupal.settings.basePath + 'node/add/' + type;
            window.location.href = path;
        });

        // 加载征集内容
        if($('.view-id-custom_facets_pages') || $('#views-form-admin-views-node-system-1')||$('#views-form-content-shenhe-list-page-1')||$('#views-form-content-shenhe-list-page-2')) {
            $('#views-form-admin-views-node-system-1, .view-custom-facets-pages').find('td.views-field-field-zjliushui').each(function(i) {
                if($(this).html().trim() != '') {
                    $(this).html('<a href="#">' + $(this).html() + '</a>').on('click', function() {
                        var code = $(this).text().trim();
                        $('#modalShowNode').modal('show');
                        $.ajax({
                            type: 'GET',
                            url: '/ajax/code-review/' + code,
                            dataType: 'html',
                            success: function(data) {
                                $('#modalShowNode').find('.modal-body').html(data).find('h2 a').attr('href', '#');
                                $('#modalShowNode').find('.modal-body .field-type-image').find('.field-item').each(function(index) {
                                    if($(this).find('a').length == 0) {
                                        var s = $(this).find('img').eq(0).attr('src');
                                        var str = s.split('/');
                                        var result= str[6];

                                        $(this).html('<a href="' + $(this).find('img').eq(0).attr('src') + '" target="_blank">'+result+'</a>');
                                    } else {

                                        var s = $(this).find('img').eq(0).attr('src');
                                        var str = s.split('/');
                                        var result= str[6];
                                        console.log(result);
                                        $(this).find('a').eq(0).html(result);
                                    }
                                    /*if(index > 0) {
                                        $(this).find('a').eq(0).html('[点击查看]');
                                    }*/
                                });
                            }
                        });
                    });
                }
            });

            $('#views-form-admin-views-node-system-1, .view-custom-facets-pages').find('td.views-field-title a').on('click', function() {
                var nid = $(this).attr('data-nid');
                if(nid == undefined) {
                    var arr = $(this).attr('href').split('/');
                    nid = arr[2];
                }
                $('#modalShowNode').modal('show');
                $.ajax({
                    type: 'GET',
                    url: '/ajax/node-review/' + nid,
                    dataType: 'html',
                    success: function(data) {
                        $('#modalShowNode').find('.modal-body').html(data).find('h2 a').attr('href', '#');
                        $('#modalShowNode').find('.modal-body .field-type-image').find('.field-item').each(function(index) {
                            if($(this).find('a').length == 0) {
                                var s = $(this).find('img').eq(0).attr('src');
                                var str = s.split('/');
                                var result= str[6];

                                $(this).html('<a href="' + $(this).find('img').eq(0).attr('src') + '" target="_blank">'+result+'</a>');
                            } else {

                                var s = $(this).find('img').eq(0).attr('src');
                                var str = s.split('/');
                                var result= str[6];
                                console.log(result);
                                $(this).find('a').eq(0).html(result);
                            }
                            /*if(index > 0) {
                                $(this).find('a').eq(0).html('[点击查看]');
                            }*/
                        });
                        var events= $('#modalShowNode').find('.modal-body').find('.field-name-field-data-source').find('.even');
                        var txt = events.text();
                        if(txt=="新著录数据")
                        {
                            $(".modal-body").children(".node").children(".content").children(".field-name-field-dc-creator").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field-bf-role").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_dc_creator4").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_bf_role1").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_bf_agent").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_bf_role2").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_bf_role3").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_bf_agent1").css("display","none");

                            $(".modal-body").children(".node").children(".content").children(".field-name-field-bf-dimensions").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field-chicungroup").find(".label").css("display","none");;
 
                        }
                        else
                        {
                            $(".modal-body").children(".node").children(".content").children(".field-name-field-chicungroup").css("display","none");
                            $(".modal-body").find(".field-name-field-colgrzzjzrfs").css("display","none");;
                            $(".modal-body").find(".field-name-field-colttzzjzrfs").css("display","none");
                            $(".modal-body").find(".field-name-field-colgrqtzrzjzrfs").css("display","none");
                            $(".modal-body").find(".field-name-field-colttqtzrzjzrfs").css("display","none");
                           
                        }
                        $(".action-links-field-collection-add").css("display","none");
                        $(".field-collection-view-links").css("display","none");
                    }
                });
                return false;
            });
             $('#views-form-content-shenhe-list-page-2,#views-form-content-shenhe-list-page-1').find('td.views-field-title a').on('click', function() {
                var nid = $(this).attr('data-nid');
                if(nid == undefined) {
                    var arr = $(this).attr('href').split('/');
                    nid = arr[2];
                }
                $('#modalShowNode').modal('show');
                $.ajax({
                    type: 'GET',
                    url: '/ajax/node-review/' + nid,
                    dataType: 'html',
                    success: function(data) {
                        $('#modalShowNode').find('.modal-body').html(data).find('h2 a').attr('href', '#');
                        $('#modalShowNode').find('.modal-body .field-type-image').find('.field-item').each(function(index) {
                            if($(this).find('a').length == 0) {
                                var s = $(this).find('img').eq(0).attr('src');
                                var str = s.split('/');
                                var result= str[6];

                                $(this).html('<a href="' + $(this).find('img').eq(0).attr('src') + '" target="_blank">'+result+'</a>');
                            } else {

                                var s = $(this).find('img').eq(0).attr('src');
                                var str = s.split('/');
                                var result= str[6];
                                console.log(result);
                                $(this).find('a').eq(0).html(result);
                            }
                            /*if(index > 0) {
                                $(this).find('a').eq(0).html('[点击查看]');
                            }*/
                        });
                        var events= $('#modalShowNode').find('.modal-body').find('.field-name-field-data-source').find('.even');
                        var txt = events.text();
                        if(txt=="新著录数据")
                        {
                            $(".modal-body").children(".node").children(".content").children(".field-name-field-dc-creator").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field-bf-role").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_dc_creator4").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_bf_role1").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_bf_agent").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_bf_role2").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_bf_role3").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field_bf_agent1").css("display","none");

                            $(".modal-body").children(".node").children(".content").children(".field-name-field-bf-dimensions").css("display","none");
                            $(".modal-body").children(".node").children(".content").children(".field-name-field-chicungroup").find(".label").css("display","none");;
 
                        }
                        else
                        {
                            $(".modal-body").children(".node").children(".content").children(".field-name-field-chicungroup").css("display","none");
                            $(".modal-body").find(".field-name-field-colgrzzjzrfs").css("display","none");;
                            $(".modal-body").find(".field-name-field-colttzzjzrfs").css("display","none");
                            $(".modal-body").find(".field-name-field-colgrqtzrzjzrfs").css("display","none");
                            $(".modal-body").find(".field-name-field-colttqtzrzjzrfs").css("display","none");
                           
                        }
                        $(".action-links-field-collection-add").css("display","none");
                        $(".field-collection-view-links").css("display","none");
                     
                    }
                });
                return false;
            });
            $('.page-search-facets .views-field-title, .page-search-facets-list .views-field-title, .page-search-creator .views-field-title, .page-search-creator-list  .views-field-title').find('a').on('click', function() {
                var nid = $(this).attr('data-nid');
                if(nid == undefined) {
                    var arr = $(this).attr('href').split('/');
                    nid = arr[2];
                }
                $('#modalShowNode').modal('show');
                $.ajax({
                    type: 'GET',
                    url: '/ajax/node-table-review/' + nid,
                    dataType: 'html',
                    success: function(data) {
                        $('#modalShowNode').find('.modal-body').html(data).find('h2 a').attr('href', '#');
                    }
                });
                return false;
            });
        }
        $('#views-form-zhengjilist-page').find('td.views-field-title a').on('click', function() {
            var nid = $(this).attr('data-nid');
            if(nid == undefined) {
                var arr = $(this).attr('href').split('/');
                nid = arr[2];
            }
            $('#modalShowNode').modal('show');
            $.ajax({
                type: 'GET',
                url: '/ajax/node-review/' + nid,
                dataType: 'html',
                success: function(data) {
                    $('#modalShowNode').find('.modal-body').html(data).find('h2 a').attr('href', '#');
                    $('#modalShowNode').find('.modal-body .field-type-image').find('.field-item').each(function(index) {
                        if($(this).find('a').length == 0) {
                            console.log('abc');
                            var s = $(this).find('img').eq(0).attr('src');
                            var str = s.split('/');
                            var result= str[6];

                            $(this).html('<a href="' + $(this).find('img').eq(0).attr('src') + '" target="_blank">'+result+'</a>');
                        } else {

                            var s = $(this).find('img').eq(0).attr('src');
                            var str = s.split('/');
                            var result= str[6];
                            console.log(result);
                            $(this).find('a').eq(0).html(result);
                        }
                        /*if(index > 0) {
                            $(this).find('a').eq(0).html('[点击查看]');
                        }*/
                    });
                }
            });
            return false;
        });

        // 选择国家和地区
        /*if($('#edit-field-bf-place-und')) {
            var place_type = $('#edit-field-bf-place-und').val();
            if(place_type == '国家' || place_type == '_none') {
                $('.field-name-field-shl-placevalue-contry').css('display', 'block');
                $('.field-name-field-shl-placevalue-area').css('display', 'none');
            } else {
                $('.field-name-field-shl-placevalue-contry').css('display', 'none');
                $('.field-name-field-shl-placevalue-area').css('display', 'block');
            }

            $('#edit-field-bf-place-und').change(function(el) {
                place_type = $('#edit-field-bf-place-und').val();
                if(place_type == '国家' || place_type == '_none') {
                    $('.field-name-field-shl-placevalue-contry').css('display', 'block');
                    $('.field-name-field-shl-placevalue-area').css('display', 'none');
                } else {
                    $('.field-name-field-shl-placevalue-contry').css('display', 'none');
                    $('.field-name-field-shl-placevalue-area').css('display', 'block');
                }
            });
        }*/

        /*$('#edit-mi-ji, #edit-operation').on('change', function() {
            if($('#edit-mi-ji').val() > 0 || $('#edit-operation').val() != 0) {
                $('#edit-submit--2').attr('disabled', false);
            }
        });*/

        // 如果设置审核未通过，必须填写原因
        if($('#edit-workbench-moderation-state-new')) {
            $current_val = $('#edit-log').val();
            $('#edit-workbench-moderation-state-new').on('change', function() {
                if($(this).val() == 'no_published') {
                    $('#edit-log').css('border-color', 'red');
                } else {
                    $('#edit-log').css('border-color', '#ccc');
                }
            });

            $('#edit-submit').on('click', function() {
                if($('#edit-workbench-moderation-state-new').val() == 'no_published') {
                    if($current_val == $('#edit-log').val()) {
                        alert('请填写审核笔记。');
                        return false;
                    }
                }
            });

            // 点击暂存，自动选择草稿状态
            $('#edit-temp-storage').on('click', function() {
                $('#edit-workbench-moderation-state-new').find('option:eq(0)').attr('selected', 'selected');
            });
            //
            $('#edit-temp-storage').parent().find('#edit-submit').on('click', function() {
                if($('#edit-workbench-moderation-state-new').val() == 'drafty') {
                    $('#edit-workbench-moderation-state-new').find('option:eq(3)').attr('selected', 'selected');
                }
            });
        }

        // 为导出按钮增加默认条件
        if($('.feed-icon').length > 0) {
            var url = $('.feed-icon').find('a').attr('href');
            if(url.indexOf('?') == -1) {
                $('.hasDatepicker').each(function(i) {
                    if(i == 0) {
                        url += '?';
                    } else {
                        url += '&';
                    }
                    url += $(this).attr('name') + '=' + $(this).val();
                });
                $('.feed-icon').find('a').attr('href', url)
            }
        }

        use_name_api_list();
        use_address_api_list();
        use_name_api_add();
        use_addr_api_add();
        set_miji();

       var va =  $("input[name='changed']").val();
       console.log(va);
       if(va=="")
       {
            $("#edit-field-chicungroup-und-0-width").val("");
            $("#edit-field-chicungroup-und-0-height").val("");
            $("#edit-field-shulianggroup-und-0-length").val("");
           
       }
       else
       {
            var txtlist =  $(".page-content").find(".draggable .even");
            txtlist.each(function(index,e){
                console.log(index+' '+$(e).html());
                var txtval = $(e).find(".text-full").val();
                if(txtval=="")
                {
                    console.log(txtval);
                    $(".draggable .even").css("display","none");
                }

            })
            
       }

       $("#field-colgrzzjzrfs-values").find(".field-multiple-drag:first").css("display","none");
       $("#field-colttzzjzrfs-values").find(".field-multiple-drag").css("display","none");
       $("#field-colgrqtzrzjzrfs-values").find(".field-multiple-drag").css("display","none");
       $("#field-colttqtzrzjzrfs-values").find(".field-multiple-drag").css("display","none");
       $("#edit-field-colgrzzjzrfs-und-0-remove-button").css("display","none");
       $("#edit-field-colttzzjzrfs-und-0-remove-button").css("display","none");
       $("#edit-field-colgrqtzrzjzrfs-und-0-remove-button").css("display","none");
       $("#edit-field-colttqtzrzjzrfs-und-0-remove-button").css("display","none");

       //实物
       $(".group-swgrzzjzrfs").css("display","none");
       $(".group-swttzzjzrfs").css("display","none");
       $(".group-swqtzrzjzrfs").css("display","none");
       $("#edit-field-bf-agent1").css("display","none");
       $("#edit-field-bf-role3").css("display","none");
       //书画篆刻
       $(".group-creatorpersonal").css("display","none");
       $(".group-contributorpersonalrole").css("display","none");
       $(".group-corporaterole").css("display","none");
       $(".group-ttqtzrz").css("display","none");
       //证件
       $(".group-zzjzrfs").css("display","none");
       $(".group-grqtzrzjzrfs").css("display","none");
       //笔记
       $(".group-bjzzjzrfs").find("#edit-field-dc-creator").css("display","none");
       $(".group-bjzzjzrfs").find("#edit-field-bf-role").css("display","none");
       //证书
       $(".group-zszzjzrfs").css("display","none");
       $(".group-zsgrqtzrz").css("display","none");
       //签名本
       $(".group-qmbgrzzjzrfs").css("display","none");
       $(".group-qmbttzzjzrfs").css("display","none");
       $("#edit-field-contributor").css("display","none");
       $("#edit-field-contributorrole").css("display","none");
       $("#edit-field-colqtzrz").find(".field-multiple-drag:first").css("display","none");
       $("#edit-field-colqtzrz-und-0-remove-button").css("display","none");
       //创作手稿
       $(".group-czsgzzjzrfs").css("display","none");
       $(".group-czsggrqtzrz").css("display","none");
       //音像资料
       $(".group-yxzlgrzzjzrfsnew1").css("display","none");
       $(".group-yxzlttzzjzrfs").css("display","none");
       $(".group-yxzltrqtzrzjzrfs").css("display","none");
       //纸质资料
       $(".group-zzzlgrzz").css("display","none");
       $(".group-zzzlttzz").css("display","none");

        //下拉框样式修改
        var lbl = $(".form-checkboxes").prev();
        lbl.addClass("lblleft");

        // 增加跳转文本框
        if($('.pager')) {
            var html = '<li>转到：<input type="number" size="3" name="pager_number" class="pager_number" /><button id="goto_button">跳转</button></li>';
            $('.pager').find('li:last-child').after(html);

            $('#goto_button').on('click', function() {
                var url = window.location.href.split('?');
                var params = url[1] == undefined ? 'page=0' : url[1];
                var p = [];
                var to_page = $('input[name="pager_number"]').val();

                params_arr = params.split('&');
                var has_page = false;
                for(var i in params_arr) {
                    p = params_arr[i].split('=');
                    if(p[0] == 'page') {
                        params_arr[i] = 'page=' + (to_page - 1);
                        has_page = true;
                    }
                }
                if(!has_page) {
                    params_arr.push('page=' + (to_page - 1));
                }

                params = params_arr.join('&');

                url = url[0] + '?' + params;
                window.location.href = url;
            });

        }
        $(".vertical-tabs-panes.vertical-tabs-processed div").each(function(i){

           var classname = this.className;
           if(i==0&&classname=="form-item")
           {
            $(this).css("display","none");
           }
           if(i==5&&classname=="form-item")
           {

                var vlas = $($(this).children("label").get(0)) ;
                vlas.css("display","none");
           }
        })
        $("#zhengji-node-form div").each(function(i){

            var classname = this.className;
            if(classname=="vertical-tabs-panes vertical-tabs-processed")
            {
                $(this).css("display","none");
            }
            
         })
        $("table tbody tr").each(function(trindex,tritem){

            var td = $(this).children();
           var oImg=td.children("img");
           var alt=oImg.attr("src");
           if(alt!=undefined)
           {
                oImg.parent().html('<a href="' + alt + '" target="_blank">[点击查看]</a>');
           }
        })

        hide_field_title();

        $("#edit-field-zjliushui-und-0-value").blur(function(){
            var lens = $('#edit-field-zjliushui-und-0-value').val();
            if(lens==""){
                    $("#edit-field-shl-donator-und-0-value").val("");
                    $("#edit-field-shl-donator1-und-0-value").val("").removeAttr("readonly");
                    $("#edit-field-dc-date2-und-0-value-datepicker-popup-0").val("").removeAttr("disabled");
                    $("#edit-field-dc-date2-und-0-value2-datepicker-popup-0").val("").removeAttr("disabled");
                    //$("#edit-field-dc-creator-und-0-value").val("");
                    //$("#edit-field-bf-extent-und-0-value").val("").removeAttr("readonly");
                    $("#edit-field-mi-ji-und").find("option").first().attr("selected", true);
                    $("#edit-field-mi-ji-und").removeAttr("disabled");

                    $(".use-api-list").removeAttr("disabled");
            }
            else
            {
                var term = lens;
                $.ajax({
                    url: '/custom_admin/fullzhengjiliushui?term='+term,
                    type: 'GET',
                    dataType:"text",
                    success: function(data) {
                      if(data!='')
                      {
                          console.log("ok");
                          $("#edit-addanother").removeAttr('disabled');
                          $("#edit-submit").removeAttr('disabled');
                          $("#edit-preview").removeAttr('disabled');
                      } 
                      else
                      {
                            var array=  term.split(')-(');
                            console.log(array.length);
                            if(array.length>1)
                            {
                                var dates = array[1].substr(0,10);
                                console.log(dates);
                                var istrue = isValidDate(dates);
                                console.log(istrue);
                                if(!isValidDate(dates))
                                {
                                    alert("未关联到征集流水，请重新填写征集流水！");
                                    $("#edit-addanother").attr('disabled','disabled');
                                    $("#edit-submit").attr('disabled','disabled');
                                    $("#edit-preview").attr('disabled','disabled');
                                }
                                else
                                {
                                    $("#edit-addanother").removeAttr('disabled');
                                    $("#edit-submit").removeAttr('disabled');
                                    $("#edit-preview").removeAttr('disabled');
                                }
                            }
                            else
                            {
                                alert("未关联到征集流水，请重新填写征集流水！");
                                $("#edit-addanother").attr('disabled','disabled');
                                $("#edit-submit").attr('disabled','disabled');
                                $("#edit-preview").attr('disabled','disabled');
                            }
                            
                            
                      }
                      
                    },
                    error:function(jqXHR){
                        console.log("Error: "+jqXHR.status);
                       
                    }
                   
                    
                });
            }
        });
     
    });
    function isValidDate(str) {
        if (!/^\d{4}\-\d\d?\-\d\d?/.test(str)) {
            return false;
        }
        var array = str.replace(/\-0/g, "-").split("-");
        var year = parseInt(array[0]);
        var month = parseInt(array[1]) - 1;
        var day = parseInt(array[2]);
        var date = new Date(year, month, day);
        return (date.getFullYear() == year && 
                date.getMonth() == month && 
                date.getDate() == day);
    }

    if(Drupal.ajax) {
        Drupal.ajax.prototype.commands.MoreUseNameApi = function(ajax, response, status) {
            use_name_api_list();
            use_address_api_list();
            use_name_api_add();
            use_addr_api_add();
            set_miji();
            hide_field_title();
        }
    }

    function hide_field_title() {
        $('.field-multiple-table').each(function(i, el) {
            var name = $(this).find('label').html();
            $(this).find('.field-multiple-drag').eq(0).html(name);
        });
    }

    // 实现批量密级设置
    function set_miji() {
        $('#edit-mi-ji, #edit-operation').on('change', function() {
            if($('#edit-mi-ji').val() > 0 || $('#edit-operation').val() != 0) {
                $('#edit-submit--2').attr('disabled', false);
            }
        });
    }

    // 展开添加规范人名
    function use_name_api_add() {
        if('.add-api-list') {

            $('#modalSearcheAddUser').on('click', function() {
                $('#modalAPIList').modal('hide');
                var current_input_id = $('#modalAPIList').find('input#current_input_index').val();
                $('#' + current_input_id).parents('.field-type-text').find('.add-api-list').click();
            });
            $('.add-api-list').on('click', function() {
                $('#modalAPIAdd').modal('show');

                var inputId = $(this).parents('.field-type-text').find('input').attr('id');
                $('#modalAPIAdd').find('input#current_add_user_input_index').val(inputId);

                $('#searchUserAddressList').on('click', function() {
                    var keyWord = $('#userAddAddress').val();
                    if(keyWord == '') {
                        alert('请输入关键字');
                        return;
                    }
                    if ($.fn.DataTable.isDataTable("#addUserAddressList")) {
                        $('#addUserAddressList').DataTable().clear().destroy();
                    }

                    $('#addUserAddressList').DataTable({
                        "processing": true,
                        "serverSide": true,
                        "ajax": {
                            url: Drupal.settings.basePath + 'get-address-api-list', // ajax source
                            dataType: 'json',
                            type: 'POST',
                            data: {search: keyWord}
                        },
                        "initComplete": function() {

                        },
                        "drawCallback": function( settings ) {
                            $('td.use_address').find('a').click(function() {
                                $('input#userAddAddress').val($(this).attr('data-fname'));
                                $('input#nativePlace').val($(this).attr('data-uri'));
                                alert('已选择当前地址');
                            });

                            $('[data-toggle="tooltip"]').tooltip();
                        },
                        //"pageLength": 10,
                        "paging": false,
                        "searching":false,
                        "info":     false,
                        "lengthChange": false,
                        "ordering": false,
                        "columns": [
                            { className: "address_country" },
                            { className: "address_province" },
                            { className: "address_city" },
                            { className: "use_address" }
                        ],
                        /*"order": [
                            [1, "desc"]
                        ],*/ // set second column as a default sort by desc
                        "language": {
                            "url" : Drupal.settings.basePath + 'sites/all/modules/custom/custom_admin/datatables/Chinese.txt'
                        }
                    });
                });

                // 点击确定后，添加内容到标准库
                $('#userSearchAPIAdd').on('click', function() {
                    var data = $('#addUserForm').serializeArray();
                    var postData = {};
                    for(var i in data) {
                        if(data[i].value == '' && data[i].name != 'deathday') {
                            title = $('#' + data[i].name).attr('placeholder');
                            alert('请输入 ' + title + ' 的值！');
                            $('#' + data[i].name).parent('div').addClass('has-error');
                            return;
                        } else {
                            $('#' + data[i].name).parent('div').removeClass('has-error');
                        }
                        if(data[i].name != 'current_add_user_input_index' && data[i].name != 'userAddAddress') {
                            postData[data[i].name] = data[i].value.replace(/\-/g, '.');
                        }
                    }
                    if(postData["birthday"]!=""&&postData["birthday"].length>4)
                    {
                        alert("出生年月长度不得大于4！");
                        return;
                    }
                    if(postData["deathday"]!=""&&postData["deathday"].length>4)
                    {
                        alert("死亡年月长度不得大于4！");
                        return;
                    }

                    if(postData["deathday"]!="")
                        {
                            var birthday =new Date(postData["birthday"]);
                            var deathday =new Date(postData["deathday"]);
                            var sysDate = new Date();

                            if(deathday<birthday){
                                alert("死亡年月必须大于出生年月！");
                                $('#' + data[i].name).parent('div').addClass('has-error');
                                return;
                            }
                            else if(sysDate<deathday) {
                                alert("死亡年月不得晚于当前日期！");
                                $('#' + data[i].name).parent('div').addClass('has-error');
                                return;
                            }
                            else {
                                $('#' + data[i].name).parent('div').removeClass('has-error');
                            }
                        }
                        else
                        {
                            var birthday=new Date(postData["birthday"]);
                            var sysDate = new Date();
                            if(sysDate<birthday) {
                                alert("出生年月不得晚于当前日期！");
                                $('#' + data[i].name).parent('div').addClass('has-error');
                                return;
                            }
                            else {
                                $('#' + data[i].name).parent('div').removeClass('has-error');
                            }
                        }
                        
                    postData.noteOfSource = '上海图书馆中国文化名人手稿知识库';
                    $.ajax({
                        url: '/add-user-to-api',
                        //url: 'http://data.library.sh.cn/persons/insert4sg',
                        type: 'POST',
                        dataType: 'JSON',
                        data: postData,
                        success: function(responseData) {
                            if(responseData.status == 'success') {
                                $('#' + data[5].value).val(data[0].value);
                                $('#modalAPIAdd').modal('hide');
                            }
                            else
                            {
                                alert("已存在该人名信息，请查询后选择！");
                                return;
                            }
                        }
                    });
                });
            });
        }
    }

    // 展开添加规范地名
    function use_addr_api_add() {
        if('.add-address-api-list') {

            $('#modalSearcheAddAddr').on('click', function() {
                $('#modalAddressAPIList').modal('hide');
                var current_input_id = $('#modalAddressAPIList').find('input#current_address_input_index').val();
                $('#' + current_input_id).parents('td').find('.add-address-api-list').click();
            });

            $('.add-address-api-list').on('click', function() {
                $('#modalAddrAdd').modal('show');

                var inputId = $(this).parents('td').find('input').attr('id');
                $('#modalAddrAdd').find('input#current_add_address_input_index').val(inputId);

                // 点击确定后，添加内容到标准库
                $('#addressAPIAdd').on('click', function() {
                    var data = $('#addAddrForm').serializeArray();
                    var postData = {};
                    for(var i in data) {
                        if(data[i].name == 'label' || data[i].name == 'country') {
                            if(data[i].value == '') {
                                title = $('#' + data[i].name).attr('placeholder');
                                alert('请输入 ' + title + ' 的值！');
                                $('#' + data[i].name).parent('div').addClass('has-error');
                                return;
                            }
                        }

                        if(data[i].name != 'current_add_address_input_index') {
                            postData[data[i].name] = data[i].value;
                        }
                    }

                    $.ajax({
                        url: '/add-addr-to-api',
                        type: 'POST',
                        dataType: 'JSON',
                        data: postData,
                        success: function(responseData) {
                            if(responseData.status == 'success') {
                                $('#' + data[5].value).val(data[0].value);
                                $('#modalAddrAdd').modal('hide');
                            }
                        }
                    });
                });
            });
        }
    }

    // 展开规范档
    function use_name_api_list() {
        if($('.use-api-list')) {
            $('.use-api-list').on('click', function() {
                $('#modalAPIList').modal('show');

                var inputId = $(this).prev().find('input').attr('id');
                $('#modalAPIList').find('input#current_input_index').val(inputId);
            });

            $('#searchAPIList').click(function() {
              update_user_name_list();
            });



            //0 means there were never any requests sent
            $('#searchKeyWord').keyup(function() {
                 if( $('#searchKeyWord').val().length>=2){ //more than 2 seconds
                    update_user_name_list();
                 }
           });
         }
    }



    function update_user_name_list(){
      var keyWord = $('#searchKeyWord').val();
      if(keyWord == '') {
      //    swal('请输入关键字。');
          return;
      }

      if ($.fn.DataTable.isDataTable("#specAuthorList")) {
          $('#specAuthorList').DataTable().clear().destroy();
      }

      $('#specAuthorList').DataTable({
          "processing": true,
          "serverSide": true,
          "ajax": {
              url: Drupal.settings.basePath + 'get-api-list', // ajax source
              dataType: 'json',
              type: 'POST',
              data: {search: keyWord}
          },
          "initComplete": function() {

          },
          "drawCallback": function( settings ) {
              $('td.use_author').find('a').click(function() {
                  var inputId = $('#modalAPIList').find('input#current_input_index').val();
                  $('input#' + inputId).val($(this).attr('data-fname'));
                  $('#modalAPIList').modal('hide');
              });
              $('td.update_user').find('a').click(function() {
                // var inputId = $('#modalAPIList').find('input#current_input_index').val();
                // $('input#' + inputId).val($(this).attr('data-fname'));
                $('#modalAPIList').modal('hide');
                $('#modalAPIupdate').modal('show');
            });

              $('[data-toggle="tooltip"]').tooltip();
          },
          "pageLength": 10,
          "searching":false,
          "info":     false,
          "lengthChange": false,
          "ordering": false,
          "columns": [
              { className: "author_name" },
              { className: "author_place" },
              { className: "author_spec" },
              { className: "use_author" },
              { className:"update_user"}

          ],
          /*"order": [
              [1, "desc"]
          ],*/ // set second column as a default sort by desc
          "language": {
              "url" : Drupal.settings.basePath + 'sites/all/modules/custom/custom_admin/datatables/Chinese.txt'
          }
      });

    }

    function use_address_api_list() {
        if($('.use-address-api-list')) {
            $('.use-address-api-list').on('click', function() {
                $('#modalAddressAPIList').modal('show');

                var inputId = $(this).prev().find('input').attr('id');
                $('#modalAddressAPIList').find('input#current_address_input_index').val(inputId);
            });

            $('#searchAddressAPIList').click(function() {
                var keyWord = $('#searchAddressKeyWord').val();
                if(keyWord == '') {
                    swal('请输入关键字。');
                    return;
                }

                if ($.fn.DataTable.isDataTable("#specAddressList")) {
                    $('#specAddressList').DataTable().clear().destroy();
                }

                $('#specAddressList').DataTable({
                    "processing": true,
                    "serverSide": true,
                    "ajax": {
                        url: Drupal.settings.basePath + 'get-address-api-list', // ajax source
                        dataType: 'json',
                        type: 'POST',
                        data: {search: keyWord}
                    },
                    "initComplete": function() {

                    },
                    "drawCallback": function( settings ) {
                        $('td.use_address').find('a').click(function() {
                            var inputId = $('#modalAddressAPIList').find('input#current_address_input_index').val();
                            $('input#' + inputId).val($(this).attr('data-fname'));
                            $('#modalAddressAPIList').modal('hide');
                        });

                        $('[data-toggle="tooltip"]').tooltip();
                    },
                    //"pageLength": 10,
                    "paging": false,
                    "searching":false,
                    "info":     false,
                    "lengthChange": false,
                    "ordering": false,
                    "columns": [
                        { className: "address_country" },
                        { className: "address_province" },
                        { className: "address_city" },
                        { className: "use_address" }
                    ],
                    /*"order": [
                        [1, "desc"]
                    ],*/ // set second column as a default sort by desc
                    "language": {
                        "url" : Drupal.settings.basePath + 'sites/all/modules/custom/custom_admin/datatables/Chinese.txt'
                    }
                });
            });
        }

      $('.views-field-field-sw-qwtp').find('a').attr('target', '_blank');
    }


    function autocomplete_liushui() {
    	if($('#edit-field-zjliushui-und-0-value').length) {
         $('#edit-field-zjliushui-und-0-value').autocomplete({
                source: "/custom_admin/zhengjiliushui",       
                select: function( event, ui ) {
                	var node = ui.item.node;
                	console.log(node);
                	var maps={
                		'field_shl_donator.und[0][value]':'edit-field-shl-donator-und-0-value',
                		'field_dc_date2.und[0][value]':'edit-field-dc-date2-und-0-value-datepicker-popup-0',
                		'field_dc_date2.und[0][value2]':'edit-field-dc-date2-und-0-value2-datepicker-popup-0',
                		'field_mi_ji.und[0][tid]':'edit-field-mi-ji-und'

                	};
                	//	个人捐赠者
                	if(typeof node.field_shl_donator.und !=='undefined' && $("#edit-field-shl-donator-und-0-value").length){
                        $("#edit-field-shl-donator-und-0-value").val(node.field_shl_donator.und[0]['value']);
                        $(".use-api-list").attr('disabled','disabled');
                	}
                    // 团体捐赠者
                	if(typeof node.field_shl_donator1.und !=='undefined' && $("#edit-field-shl-donator1-und-0-value").length){
                		$("#edit-field-shl-donator1-und-0-value").val(node.field_shl_donator1.und[0]['value']).attr('readonly','readonly');
                    }
                	// 捐赠日期
                	if(typeof node.field_donated.und[0]['value'] !=='undefined' && $("#edit-field-dc-date2-und-0-value-datepicker-popup-0").length){
                        var dt = node.field_donated.und[0]['value'].substr(0,10);
                        $("#edit-field-dc-date2-und-0-value-datepicker-popup-0").val(dt).attr('disabled','disabled');
                		$("#edit-field-dc-date2-und-0-value2-datepicker-popup-0").attr('disabled','disabled');
                    }
                	// 个人作者
                	// if(typeof node.field_dc_creator.und[0]['value'] !=='undefined' && $("#edit-field-dc-creator-und-0-value").length){
                	// 	$("#edit-field-dc-creator-und-0-value").val(node.field_dc_creator.und[0]['value']);
                    // }
                    // else
                    // {
                    //     $("#edit-field-dc-creator-und-0-value").removeAttr("readonly");
                    // }

                	// 数量
                	// if(typeof node.field_bf_extent.und[0]['value'] !=='undefined' && $("#edit-field-bf-extent-und-0-value").length){
                	// 	$("#edit-field-bf-extent-und-0-value").val(node.field_bf_extent.und[0]['value']).attr('readonly','readonly');
                    // }
                    // else{
                    //     $("#edit-field-bf-extent-und-0-value").removeAttr("readonly");
                    // }

                	//密级
                	// if(typeof node.field_mi_ji.und[0]['tid'] !=='undefined'  && $("#edit-field-mi-ji-und").length){       
                	// 	$("#edit-field-mi-ji-und").val(node.field_mi_ji.und[0]['tid']).attr('disabled','disabled');                		;                		
                    // } 
                    
                	
                }  
            });
       }

    }







})(jQuery);
