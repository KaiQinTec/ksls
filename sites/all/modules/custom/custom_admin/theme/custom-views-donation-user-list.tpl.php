<style type="text/css">
    .view-filters {
        margin-top: 20px;
    }

    .user-lists {
        width: 100%;
        margin-top: 20px;
    }

    .user-lists li {
        float: left;
    }

    .user-lists li a {
        font-size: 16px;
        line-height: 32px;
    }
</style>
<?php 
if($type == 'donation') {
    $title = '捐献者';
} elseif($type == 'creator') {
    $title = '责任者';
}
?>
<div class="view-filters">
    <form action="?" method="get" id="" class="form-inline" accept-charset="UTF-8"><div>
        <div class="form-group">
            <label for="edit-field-shl-donator"><?php print $title;?>姓名</label>
            <input class="form-control form-text" type="text" id="edit-field-shl-donator" name="field_shl_donator" value="<?php print isset($_GET['field_shl_donator']) ? $_GET['field_shl_donator'] : '';?>" size="30" maxlength="128">
        </div>
        <div class="form-group">
            <input class="btn btn-default form-submit" type="submit" id="edit-submit-custom-facets-pages" name="" value="查询">
        </div>
    </form>    
</div>

<div class="user-lists">
    <table class="table table-bordered">


        <thead>
            <tr>
                <td><?php print $title;?>姓名</td>
                <td>简介</td>
                <td>
                    <?php
              if($type == 'donation') {
                print '捐献总量';
              } elseif($type == 'creator') {
                print '创作总量';
              }?>
                </td>
                <td>
                  <?php
                  if($type == 'donation') {
                    print '捐献批数';
                  } elseif($type == 'creator') {
                    print '创作种类';
                  }?>
                </td>
                <td>
                  <?php
                  if($type == 'donation') {
                    print  '首次捐献时间';
                  } elseif($type == 'creator') {
                    print '首次创作时间';
                  }?>
                </td>
                <td>
                  <?php
                  if($type == 'donation') {
                    print '最近捐献时间';
                  } elseif($type == 'creator') {
                    print '最近创作时间';
                  }?>
                </td>
            </tr>

        </thead>
        <tbody>
            <?php foreach($users as $user) : ?>
            <?php
            $name = isset($user->field_shl_donator_value) ? $user->field_shl_donator_value : $user->field_dc_creator_value;
            if($type == 'donation') {
                $url = 'search/facets';
                $field = 'field_shl_donator';
            } elseif($type == 'creator') {
                $url = 'search/creator';
                $field = 'field_dc_creator';
            }
            ?>
            <tr>
                <td>
                    <a href="<?php print url($url, array('query' => [$field => $name]));?>">
                        <?php print $name;?>
                    </a>
                </td>
                <td></td>
                <td><?php print $user->count;?></td>
                <td><?php print $user->line_count;?></td>
                <td><?php print $user->start;?></td>
                <td><?php print $user->end;?></td>
            </tr>
            <?php endforeach;?>
        </tbody>
    </table>
</div>
<div class="user-list-pager">
    <?php print $pager;?>
</div>