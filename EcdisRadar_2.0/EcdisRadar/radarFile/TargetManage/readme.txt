一、点迹管理
1、点迹结构管理
2、点迹类型管理
3、点迹显示管理

二、航迹管理：
1、从跟踪单元接收到航迹相关报文后的处理
    setTrackInfo:创建新航迹或更新航迹位置参数
    setTrackStatus:设置航迹状态
    setTrackManInit:人工航迹起始
    setTrackTypeAttr:设置目标类型属性
    setChgTrackNo:改换批处理
    setTrackDelete:航迹目标删除
    (为了减少内存，航迹结构可以采取动态申请和管理)

2、航迹位置更新处理
    updateScreenPoint:更新航迹屏幕坐标(视图变化或量程变化时调用)
    updateSquareAndScreenPoint:由经纬度坐标计算直角坐标、极坐标、屏幕坐标(量纲变化时调用)
    (move:目标位置移动处理，直角坐标、屏幕坐标平移，并重新计算极坐标、经纬度坐标)
    centerMove:中心点移动，需要移动目标直角坐标、屏幕坐标，并重新计算极坐标，目标经纬度坐标不变
    trackMove:需要移动目标直角坐标、屏幕坐标，并重新计算极坐标、经纬度坐标

3、航迹类型管理
    实现航迹类型动态管理，可动态注册，并设置各项参数，包括颜色、显示控制标志、名称、符号、符号大小
    registerType / unregisterType:
    setTypeFlag/setTrackColor/setTrackTypeName/setTrackTypeSymbol/setTrackTypeSymbolSize
    isTypeValid:

4、标牌指引线管理
    标牌内容：批号、方位距离、速度航向、高度、3A代码、多普勒速度等
    标牌模式：无标牌(不显)
            单标牌(显1行，固定批号)
            简标牌(显3行，第1行为固定批号，第2行为固定但显示内容可设置，第3行为可切换显示)
            全标牌(显5行，第1行为固定批号，第2到4行固定，但显示内容可设置，第5行可切换显示)
    字体大小：小号、中号、大号、超大
    指引线长度模式：固定、自动、人工
    指引线长度：固定(分为7级可调)、自动(由速度时间来计算)、人工(人工拖动)
    指引线方向：航向
    初始化时调用
    setDefaultBoardLineValue:指定不同模式的默认标牌每行显示内容
    registerBoardLineValue:注册可用的标牌显示内容索引值
    人工操作调用
    setTrackBoardMode:设置航迹标牌显示模式(或者系统标牌显示模式)
    switchBoardLastLine:切换指定航迹标牌最后一行显示内容
    formBoardText:输出标牌某一行字符串，按照该行指定的显示内容索引值

5、航迹尾点管理
    包含3种操作规程：更改系统尾点模式、更改航迹尾点模式、人工设置航迹尾点数量
    registerTrackPointMode:注册航迹尾点模式
    setTrackPointMode:更改系统尾点模式、更改航迹尾点模式
    setTrackPointSize:人工设置航迹尾点数量
    trackPointMode/trackPointModeName:

6、航迹显示管理

7、标牌自动避让管理

三、雷达视图管理
    显示模式管理(P显、B显、A显)、坐标转换、
    P显操作管理(放大、漫游、空心、延迟、偏心)
    B显操作管理(放大、漫游):setBScanRange/setBScanAzimuth
    绘制方位距离刻度线、绘制经纬度刻度线、绘制九九五五方格

    需要管理的参数:视图屏幕区域、雷达中心点坐标(经纬度坐标、屏幕坐标)、量程(象素距离比例)、显示模式、偏心标志、漫游标志、放大标志、
    可变参数：设置视图屏幕区域、设置雷达中心点屏幕坐标、设置量程、设置显示模式、设置偏心漫游放大标志
    窗口大小改变时，设置视图屏幕区域；偏心放大漫游时，设置雷达中心点屏幕坐标和量程；

