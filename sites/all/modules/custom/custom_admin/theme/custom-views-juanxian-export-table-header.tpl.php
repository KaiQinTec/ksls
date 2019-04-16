<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <!--[if gte mso 9]>
    <xml>
    <x:ExcelWorkbook>
    <x:ExcelWorksheets>
    <x:ExcelWorksheet>
    Name of the sheet
    <x:WorksheetOptions>
    <x:Panes>
    </x:Panes>
    </x:WorksheetOptions>
    </x:ExcelWorksheet>
    </x:ExcelWorksheets>
    </x:ExcelWorkbook>
    </xml>
    <![endif]-->
  </head>
  <body>
    <table>
        <thead>
        <?php foreach ($header as $field => $label): ?>
            <?php if($label == '审核') continue;?>
            <th <?php if($label != '名称') echo 'colspan="2"';?> <?php if ($header_classes[$field]): ?> class="<?php print $header_classes[$field]; ?>"<?php endif; ?> scope="col">
                <?php print $label; ?>
            </th>
        <?php endforeach; ?>
        </thead>
    <?php print $header_row; ?>
    <tbody>