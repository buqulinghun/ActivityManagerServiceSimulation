#include "conditionalitem.h"
#include "ecdis.h"

extern MARINERSELECT MarinerSelect;



ConditionalItem::ConditionalItem()
{
    nowChart = NULL;
    setupConditionalNameMap();

}



/***********************条件物标处理接口******************************/
//处理区域型条件物标
QStringList ConditionalItem::dealConditionalAreaItem(const QString &name, std::vector<AreaObject>::iterator &object) const
{
    const int conditionalName = conditional_name_num_map.value(name);
    QStringList symbolInstruction;
    switch(conditionalName) {
        case DATCVR01:    symbolInstruction = setup_DATCVR01(object);  break;
        case DEPARE01:    symbolInstruction = setup_DEPARE01(object);  break;
        case RESARE02:    symbolInstruction = setup_RESARE02(object);  break;
        case RESTRN01:    symbolInstruction = setup_RESTRN01(object);  break;
        case WRECKS02:    symbolInstruction = setup_WRECKS02_area(object);  break;
        case OBSTRN04:    symbolInstruction = setup_OBSTRN04_area(object);  break;
    }
    return symbolInstruction;
}

//处理线型条件物标
QStringList ConditionalItem::dealConditionalLineItem(const QString &name, std::vector<LineObject>::iterator &object) const
{
    const int conditionalName = conditional_name_num_map.value(name);
    QStringList symbolInstruction;
    switch(conditionalName) {
        case DATCVR01:    symbolInstruction = setup_DATCVR01(object);  break;
        case DEPCNT02:    symbolInstruction = setup_DEPCNT02(object);    break;
        case QUAPOS01:    symbolInstruction = setup_QUAPOS01_Line(object);   break;
        case OBSTRN04:    symbolInstruction = setup_OBSTRN04_line(object);  break;
    }
    return symbolInstruction;
}

//处理点条件物标
QStringList ConditionalItem::dealConditionalPointItem(const QString &name, std::vector<PointObject>::iterator &object) const
{
    const int conditionalName = conditional_name_num_map.value(name);
    QStringList symbolInstruction;
    switch(conditionalName) {
        case DATCVR01:    symbolInstruction = setup_DATCVR01(object);  break;
        case QUAPOS01:    symbolInstruction = setup_QUAPOS01_Point(object);   break;
        case LIGHTS05:    symbolInstruction = setup_LIGHTS05(object);  break;
        case WRECKS02:    symbolInstruction = setup_WRECKS02_point(object);  break;
        case TOPMAR01:    symbolInstruction = setup_TOPMAR01(object);  break;
        case OBSTRN04:    symbolInstruction = setup_OBSTRN04_point(object);  break;
    }
    return symbolInstruction;
}


//*********************************区域型条件物标处理********************************
QStringList ConditionalItem::setup_DATCVR01(const std::vector<AreaObject>::iterator &object) const
{
    //描述的不是太清除，后面有时间再弄
    QStringList symbolIns;
    return symbolIns;
}
QStringList ConditionalItem::setup_DEPARE01(const std::vector<AreaObject>::iterator &object) const
{
    QStringList symbolIns;   //存储符号化语句
    float drval1=-1, drval2=-1;
    //获取DRVAL1和DRVAL2的属性值,没有给则分别赋值-1m和DRVAL1+0.01m,先有87属性再88属性
    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = object->attfs.end();
    for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
        if((attf_it->attl == 87)) {   //DRVAL1
            const QString at = QString(attf_it->atvl.c_str());   //物标属性值
            if(! at.isEmpty())
                drval1 = at.toFloat();
        }

        if((attf_it->attl == 88)) {   //DRVAL2
            const QString at = QString(attf_it->atvl.c_str());
            if(! at.isEmpty())
                drval2 = at.toFloat();
        }
    }
    if(drval2 < 0)   //没有给drval2属性
        drval2 = drval1 + 0.01;

    //调用SEABED01('DRVAL1', 'DRVAL2'),该程序返回一个填充颜色值以及是否填充模型DIAMOND1
    bool fillPattern = false;
    const QString fill = setup_SEABED01(drval1, drval2, fillPattern);
    symbolIns.append(QString("AC(%1)").arg(fill));
    if(fillPattern)
        symbolIns.append(QString("AP(DIAMOND1)"));

    //检测该物标是否为DRGARE
    if(object->object_label == DRGARE) {
        symbolIns.append(QString("AP(DRGARE01)"));    //填充图案DRGARE01
        symbolIns.append(QString("LS(DASH,1,CHGRF)"));   //绘制边界LS(DASH,1,CHGRF)
        for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
            if((attf_it->attl == 131)) {    //获取属性RESTRN
                const QString at = QString(attf_it->atvl.c_str());   //物标属性值
                if(! at.isEmpty()){           
                    const QString centerName = setup_RESCSP02(at);   //调用其他程序RESCSP02(RESTRN),就是选择一个符号标记显示在区域中心,函数返回需要显示的符号名称
                    symbolIns.append(QString("SY(%1)").arg(centerName));
                }
                break;
            }
        }  //end for
    }

    //附加部分A处理
    //取该面物标的每个引用的空间物标循环处理，检测该空间物标是否被DEPCNT物标引用，该DEPCNT物标的属性VALDCO获取
    //该空间物标是否为另外一个DEPARE或者DRGARE引用，还有被其他物标引用，所以是不是要在该空间物标中记录被谁引用？还要记录引用它的DEOPCNT的VALDCO属性？？？？
    std::vector<fspt_t>::iterator fit;
    std::vector<fspt_t>::iterator fend = object->fspts.end();
    for(fit = object->fspts.begin(); fit != fend; ++fit) {
        int ref_record = fit->rcid;
        bool safe = false;
        bool unsafe = false;
        bool loc_safety = false;
        float loc_valdco = -1;   //表示unknow
        if(drval1 < MarinerSelect.SAFETY_CONTOUR)
            unsafe = true;
        else
            safe = true;

        //检测该物标是否被DEPCNT物标引用
        std::map<int, std::vector<LineObject> >::iterator lom_it;
        std::map<int, std::vector<LineObject> >::iterator lom_end = nowChart->line_objects_map.end();
        for(lom_it = nowChart->line_objects_map.begin(); lom_it != lom_end; ++lom_it) {
            if(lom_it->first == DEPCNT) {
                bool find_flag = false;
                std::vector<LineObject>::iterator lo_it;
                std::vector<LineObject>::iterator lo_end = lom_it->second.end();
                for(lo_it = lom_it->second.begin(); lo_it != lo_end; ++lo_it) {
                    std::vector<fspt_t>::iterator fit;
                    std::vector<fspt_t>::iterator fend = lo_it->fspts.end();
                    for(fit = lo_it->fspts.begin(); fit != fend; ++fit) {
                        if(fit->rcid == ref_record){    //有相同引用空间物标
                            loc_valdco = 0.0;
                            std::vector<attf_t>::iterator attf_it;
                            std::vector<attf_t>::iterator attf_end = lo_it->attfs.end();
                            for(attf_it = lo_it->attfs.begin(); attf_it != attf_end; ++ attf_it){
                                if((attf_it->attl == 174)) {   //VALDCO
                                    loc_valdco = QString(attf_it->atvl.c_str()).toFloat();  //没有为0.0
                                    break;
                                }
                            }
                            find_flag = true;
                            break;
                        }
                    }
                    if(find_flag)  break;
                }
                break;
            }
        }


        if(loc_valdco == MarinerSelect.SAFETY_CONTOUR)   //loc_valdco是否等于水深线
            loc_safety = true;
        else{
            //检测是否被另外的DEPARE和DRGARE共用
            bool find_flag = false;
            std::map<int, std::vector<AreaObject> >::iterator aom_it;
            std::map<int, std::vector<AreaObject> >::iterator aom_end = nowChart->area_objects_map.end();
            for(aom_it = nowChart->area_objects_map.begin(); aom_it != aom_end; ++aom_it) {
                if(lom_it->first == DEPARE || lom_it->first == DRGARE) {
                    std::vector<AreaObject>::iterator ao_it;
                    std::vector<AreaObject>::iterator ao_end = aom_it->second.end();
                    for(ao_it = aom_it->second.begin(); ao_it != ao_end; ++ao_it) {
                        if(ao_it->record_id == object->record_id)    continue;
                        std::vector<fspt_t>::iterator fit;
                        std::vector<fspt_t>::iterator fend = ao_it->fspts.end();
                        for(fit = ao_it->fspts.begin(); fit != fend; ++fit) {
                            if(fit->rcid == ref_record){    //有相同引用空间物标
                                float drval_1 = -1;
                                std::vector<attf_t>::iterator attf_it;
                                std::vector<attf_t>::iterator attf_end = ao_it->attfs.end();
                                for(attf_it = ao_it->attfs.begin(); attf_it != attf_end; ++ attf_it){
                                    if((attf_it->attl == 87)) {   //DRVAL1
                                        drval_1 = QString(attf_it->atvl.c_str()).toFloat();
                                        if(drval_1 == 0.0)   drval_1 = -1;
                                        break;
                                    }
                                }
                                if(drval_1 < MarinerSelect.SAFETY_CONTOUR)
                                    unsafe = true;
                                else
                                    safe = true;
                                find_flag = true;
                                break;
                            }
                        }
                        if(find_flag)  break;
                    }  //end areaobject
                    break;
                }
            }
            if(!find_flag) {   //没有被另外的DEPARE和DRGARE共用
                //是否被LNDARE或UNSARE物标引用,是的话是否被RIVERS LAKARE CANALS或 DOCARE引用，是的话 unsafe =true

            }
        }

        if(!loc_safety) {
            if(unsafe && safe)
                loc_safety = true;
        }
        //自定义一种符号化语句，绘制线段LZ(SOLD,2,DEPSC,OVERRADAR,Priority,index_edge,),没有比例尺限制
        if(loc_safety) {
            //检测有没有QUAPOS属性,一般都没有,可以去掉
            int quapos = 0;
            std::map<int, EdgeVector>::iterator evm_it;
            evm_it = nowChart->edge_vectors_map.find(ref_record);
            const int size = evm_it->second.attvs.size();
            for(int i=0; i<size; i++) {
                if(evm_it->second.attvs[i].attl == 402) {
                    quapos = QString(evm_it->second.attvs[i].atvl.c_str()).toInt();
                }
            }
            if((quapos > 1) && (quapos < 10))
                symbolIns.append(QString("LZ(DASH,2,DEPSC,1,8,%1,)").arg(ref_record));
            else
                symbolIns.append(QString("LZ(SOLD,2,DEPSC,1,8,%1,)").arg(ref_record)); //LS(SOLD, 2, DEPSC), OVERRADAR=true,viewGroup = 13010,priority=8,任何比例尺都要绘出来

            //如果选择显示SAFETY CONTOUR LABELS,执行下面程序
            if(!(loc_valdco == -1)) {
                //调用SAFCON01程序
            }
        }

    }

    return symbolIns;
}

QString ConditionalItem::setup_SEABED01(float &drval1, float &drval2, bool &fill) const
{
    bool SHALLOW = true;
    QString color;
    if(MarinerSelect.TWO_SHADES){   //水深显示两个分界区域
        //获取颜色DEPIT,或者后面更改为DEPVS,DEPDW
        color = QString("DEPIT");
        if((drval1 >= 0) && (drval2 > 0))
            color = QString("DEPVS");
        if((drval1 >= MarinerSelect.SAFETY_CONTOUR) && (drval2 >MarinerSelect.SAFETY_CONTOUR)) {
            color = QString("DEPDW");
            SHALLOW = false;
        }
    }else{
        color = QString("DEPIT");
        if((drval1 >= 0) && (drval2 > 0))
            color = QString("DEPVS");
        if((drval1 >= MarinerSelect.SHALLOW_CONTOUR) && (drval2 >MarinerSelect.SHALLOW_CONTOUR))
            color = QString("DEPMS");
        if((drval1 >= MarinerSelect.SAFETY_CONTOUR) && (drval2 >MarinerSelect.SAFETY_CONTOUR)) {
            color = QString("DEPMD");
            SHALLOW = false;
        }
        if((drval1 >= MarinerSelect.DEEP_CONTOUR) && (drval2 >MarinerSelect.DEEP_CONTOUR)) {
            color = QString("DEPDW");
            SHALLOW = false;
        }
    }

    if(SHALLOW && MarinerSelect.SHALLOW_PATTERN)
        fill = true;
    return color;
}

QStringList ConditionalItem::setup_RESTRN01(const std::vector<AreaObject>::iterator &object) const
{  
    QStringList symInsu;
    QString restrn;   //获取属性RESTRN
    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = object->attfs.end();
    for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
        if((attf_it->attl == 131)) {   //RESTRN
            restrn = QString(attf_it->atvl.c_str());   //物标属性值
        }
    }

    if(! restrn.isEmpty()) {
        symInsu.append(QString("SY(%1)").arg(setup_RESCSP02(restrn)));
    }
    return symInsu;
}


bool CheckContain(const QString &string, const QStringList &list)
{
    const QStringList str = string.split(",");
    for(int i=0; i< str.size(); i++) {
        if(list.contains(str.at(i))) {
            return true;
        }
    }
    return false;
}
QString ConditionalItem::setup_RESCSP02(const QString &restrn) const
{
    QString symbol;
    //通过属性值中是否含有相应的项来显示，属性值应该是用，分开的
    QStringList res = restrn.split(",");
    if(CheckContain("7,8,14", res)) {
        if(CheckContain("1,2,3,4,5,6,13,16,17,23,24,25,26,27", res)) {
            symbol = "ENTRES61";
        }else {
            if(CheckContain("9,10,11,12,15,18,19,20,21,22", res)) {
                symbol = "ENTRES71";
            }else {
                symbol = "ENTRES51";
            }
        }
    }else if(CheckContain("1,2", res)) {
        if(CheckContain("3,4,5,6,13,16,17,23,24,25,26,27", res)) {
            symbol = "ACHRES61";
        }else {
            if(CheckContain("9,10,11,12,15,18,19,20,21,22", res)) {
                symbol = "ACHRES71";
            }else {
                symbol = "ACHRES51";
            }
        }
    }else if(CheckContain("3,4,5,6,24", res)) {
        if(CheckContain("13,16,17,23,25,26,27", res)) {
            symbol = "FSHRES61";
        }else {
            if(CheckContain("9,10,11,12,15,18,19,20,21,22", res)) {
                symbol = "FSHRES71";
            }else {
                symbol = "FSHRES51";
            }
        }
    }else if(CheckContain("13,16,17,23,25,26,27", res)) {
        if(CheckContain("9,10,11,12,15,18,19,20,21,22", res)) {
            symbol = "CTYARE71";
        }else {
            symbol = "CTYARE51";
        }
    }else if(CheckContain("9,10,11,12,15,18,19,20,21,22", res)) {
        symbol = "INFARE51";
    }else {
        symbol = "RSRDEF51";
    }
    return symbol;
}

QStringList ConditionalItem::setup_RESARE02(const std::vector<AreaObject>::iterator &object) const
{
    QStringList symbolIns;
    //获取属性RESTRN和CATREA
    QStringList restrn,catrea;
    QString centerSymbol;

    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = object->attfs.end();
    for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
        if(attf_it->attl == 56) {   //CATREA
            const QString at = QString(attf_it->atvl.c_str());
            catrea = at.split(",");
        }
        if(attf_it->attl == 131) {  //RESTRN
            const QString at = QString(attf_it->atvl.c_str());
            restrn = at.split(",");
            break;
        }
     }

    if(! restrn.isEmpty()) {
        if(CheckContain("7,8,14", restrn)) {
             //condition A///////////////////////////////
            if(CheckContain("1,2,3,4,5,6,13,16,17,23,24,25,26,27", restrn))
                centerSymbol = "ENTRES61";
            else {
                if(CheckContain("1,8,9,12,14,18,19,21,24,25,26", catrea))
                    centerSymbol = "ENTRES61";
                else {
                    if(CheckContain("9,10,11,12,15,18,19,20,21,22", restrn))
                        centerSymbol = "ENTRES71";
                    else {
                        if(CheckContain("4,5,6,7,10,20,22,23", catrea))
                            centerSymbol = "ENTRES71";
                        else
                            centerSymbol = "ENTRES51";
                    }
                }
            }
            object->priority = 6;   //只有这一个符号化指令，所以不会影响其他的，就一次性赋值
            if(! MarinerSelect.plainArea)                             //符号化边界,LC(CTYARE51)
                symbolIns.append(QString("LC(CTYARE51)"));
            else  //LS(DASH,2,CHMGD)
                symbolIns.append(QString("LS(DASH,2,CHMGD)"));

            symbolIns.append(QString("SY(%1)").arg(centerSymbol));   //显示中心物标
            return symbolIns;
         }else if(CheckContain("1,2", restrn)) {
                //condition B///////////////////////////////
                if(CheckContain("3,4,5,6,13,16,17,23,24,25,26,27", restrn))
                    centerSymbol = "ACHRES61";
                else {
                    if(CheckContain("1,8,9,12,14,18,19,21,24,25,26", catrea))
                        centerSymbol = "ACHRES61";
                    else {
                        if(CheckContain("9,10,11,12,15,18,19,20,21,22", restrn))
                            centerSymbol = "ACHRES71";
                        else {
                            if(CheckContain("4,5,6,7,10,20,22,23", catrea))
                                centerSymbol = "ACHRES71";
                            else
                                centerSymbol = "ACHRES51";
                        }
                    }
                }
                object->priority = 6;
                if(! MarinerSelect.plainArea)     //符号化边界,LC(ACHRES51)
                    symbolIns.append(QString("LC(ACHRES51)"));
                else   //LS(DASH,2,CHMGD)
                    symbolIns.append(QString("LS(DASH,2,CHMGD)"));

                symbolIns.append(QString("SY(%1)").arg(centerSymbol));
                return symbolIns;

        }else if(CheckContain("3,4,5,6,24", restrn)) {
                    //condition C///////////////////////////////
                    if(CheckContain("13,16,17,23,24,25,26,27", restrn))
                        centerSymbol = "FSHRES61";
                    else {
                        if(CheckContain("1,8,9,12,14,18,19,21,24,25,26", catrea))
                            centerSymbol = "FSHRES61";
                        else {
                            if(CheckContain("9,10,11,12,15,18,19,20,21,22", restrn))
                                centerSymbol = "FSHRES71";
                            else {
                                if(CheckContain("4,5,6,7,10,20,22,23", catrea))
                                    centerSymbol = "FSHRES71";
                                else
                                    centerSymbol = "FSHRES51";
                            }
                        }
                    }

                    object->priority = 6;
                    if(! MarinerSelect.plainArea)   //符号化边界,LC(FSHRES51)
                       symbolIns.append(QString("LC(FSHRES51)"));
                    else   //LS(DASH,2,CHMGD)
                        symbolIns.append(QString("LS(DASH,2,CHMGD)"));
                    symbolIns.append(QString("SY(%1)").arg(centerSymbol));

                    return symbolIns;
        }else if(CheckContain("13,16,17,23,25,26,27", restrn)) {
            //condition D///////////////////////////////
            if(CheckContain("9,10,11,12,15,18,19,20,21,22", restrn))
                centerSymbol = "CTYARE71";
            else {
                if(CheckContain("4,5,6,7,10,20,22,23", catrea))
                    centerSymbol = "CTYARE71";
                else
                    centerSymbol = "CTYARE51";
            }
            object->priority = 6;
            if(! MarinerSelect.plainArea)
               symbolIns.append(QString("LC(CTYARE51)"));
            else
               symbolIns.append(QString("LS(DASH,2,CHMGD)"));
            symbolIns.append(QString("SY(%1)").arg(centerSymbol));

            return symbolIns;

       }else {
           if(CheckContain("9,10,11,12,15,18,19,20,21,22", restrn))
               centerSymbol = "INFARE51";
           else
               centerSymbol = "RSRDEF51";

           if(! MarinerSelect.plainArea)
              symbolIns.append(QString("LC(CTYARE51)"));
           else
               symbolIns.append(QString("LS(DASH,2,CHMGD)"));
            symbolIns.append(QString("SY(%1)").arg(centerSymbol));

           return symbolIns;
       }

    }else {   //condition E 处理
        if(! catrea.isEmpty()) {
            if(CheckContain("1,8,9,12,14,18,19,21,24,25,26", catrea)) {
                if(CheckContain("4,5,6,7,10,20,22,23", catrea))
                    centerSymbol = "CTYARE71";
                else
                    centerSymbol = "CTYARE51";
            }else {
                if(CheckContain("4,5,6,7,10,20,22,23", catrea))
                    centerSymbol = "INFARE51";
                else
                    centerSymbol = "RSRDEF51";
            }
        }else
            centerSymbol = "RSRDEF51";

        if(! MarinerSelect.plainArea)
           symbolIns.append(QString("LC(CTYARE51)"));
        else
            symbolIns.append(QString("LS(DASH,2,CHMGD)"));
         symbolIns.append(QString("SY(%1)").arg(centerSymbol));

         return symbolIns;
    }

}


QStringList ConditionalItem::setup_WRECKS02_area(std::vector<AreaObject>::iterator &object) const
{
    QStringList symbolIns;
    float depthValue = -1000;
    float leastDepth = -1000;
    float seabedDepth = -1000;
    float valsou = -1000;
    int catwrk = -1;
    int watlev = -1;
    int expsou = -1;
    QStringList symbol;

    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = object->attfs.end();
    for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
        if(attf_it->attl == 71) {   //CATWRK
            catwrk = QString(attf_it->atvl.c_str()).toInt();  continue;
        }
        if(attf_it->attl == 93) {  //EXPSOU
            expsou = QString(attf_it->atvl.c_str()).toInt();  continue;
        }
        if(attf_it->attl == 179) {  //VALSOU
            const QString at = QString(attf_it->atvl.c_str());
            valsou = at.toFloat();
            continue;
        }
        if(attf_it->attl == 187) {   //WATLEV
            watlev = QString(attf_it->atvl.c_str()).toInt();   continue;
        }

    }
    if((valsou != -1000) && (valsou != 0.0)) {   //将viewgroup变为34051,没用这个,调用SNDFRM03(depthValue)条件程序,即显示该深度值出来
        //面物标应该没有valsou这个属性，所以也不会执行下面的程序
        int quapos = -1;   //位置精度
        depthValue = valsou;
        /*if((attv_it->attl == 402)) {   //QUAPOS
            quapos = QString(attv_it->atvl.c_str()).toInt();
        } */
        symbol = setup_SNDFRM03(depthValue, quapos, (object->attfs));
    }else {   //没有给出VALSOU属性值
         //调用DEPVAL02条件程序,以这两个属性值为参数,返回leastDepth,seabedDepth
         setup_DEPVAL02_area(object, expsou, watlev, leastDepth, seabedDepth);
         if(leastDepth == -1000) {
             if(catwrk > 0) {  //有CATWRK属性
                 if(catwrk == 1) {
                     depthValue = 20.1;
                     if(seabedDepth != -1000) {
                         leastDepth = seabedDepth - 66.0;
                         if(! (leastDepth < 20.1)) {
                             depthValue = leastDepth;
                         }
                     }
                 }else
                     depthValue = -15;
             }else{
                 if(watlev > 0) {
                     if((watlev == 3) || (watlev == 5))
                         depthValue = 0;
                     else
                         depthValue = -15;
                 }else
                     depthValue = -15;
             }
         }else
             depthValue = leastDepth;
     }

    //区域物标
    //调用UDWHAZ04('depthValue'),得到是否显示isolated danger符号,SY(ISODGR01)
    bool danger = setup_UDWHAZ04_area(object, depthValue, watlev);
    symbolIns.append(setup_QUAPNT02_area(object));      //调用QUAPNT02条件程序得到是否显示LOW ACCURACY符号

    //condition B///////////循环检测每个引用空间物标
    std::vector<fspt_t>::iterator fit;
    std::vector<fspt_t>::iterator fend = object->fspts.end();
    for(fit = object->fspts.begin(); fit != fend; ++fit) {
        int ref_record = fit->rcid;
        int quapos = -1;
        std::map<int, EdgeVector>::iterator evm_it;
        evm_it = nowChart->edge_vectors_map.find(ref_record);
        const int size = evm_it->second.attvs.size();
        for(int i=0; i<size; i++) {
            if(evm_it->second.attvs[i].attl == 402) {
                quapos = QString(evm_it->second.attvs[i].atvl.c_str()).toInt();
            }
        }
        if(quapos != -1) {  //有该属性
            if((quapos > 1) && (quapos < 10)) {  //LC(LOWACC41)绘制该条线段
                symbolIns.append(QString("LC(LOWACC41,%1,)").arg(ref_record));
                continue;
            }
        }

        //每个空间引用物标的处理，加上引用线段号//////////////
        if(danger) {
            symbolIns.append(QString("LS(DOTT,2,CHBLK,%1,)").arg(ref_record));
        }else {
            if(valsou > 0) {
                if(valsou <= 20)
                    symbolIns.append(QString("LS(DOTT,2,CHBLK,%1,)").arg(ref_record));
                else
                    symbolIns.append(QString("LS(DASH,2,CHBLK,%1,)").arg(ref_record));
            }else {
                if((watlev == 1) || (watlev == 2))
                    symbolIns.append(QString("LS(SOLD,2,CSTLN,%1,)").arg(ref_record));
                else if(watlev == 4)
                    symbolIns.append(QString("LS(DASH,2,CSTLN,%1,)").arg(ref_record));
                else
                    symbolIns.append(QString("LS(DOTT,2,CSTLN,%1,)").arg(ref_record));
            }
        }

    }

    if(valsou > 0) {
        if(danger)
            symbolIns.append(QString("SY(ISODGR01)"));
        else {
            for(int i=0; i < symbol.size(); i++) {
                symbolIns.append(QString("SY(%1)").arg(symbol.at(i)));   //返回的是不带SY标志的符号，需要加上
            }
        }
    }else {
        if((watlev == 1) || (watlev == 2))
            symbolIns.append(QString("AC(CHBRN)"));
        else if(watlev == 4)
            symbolIns.append(QString("AC(DEPIT)"));
        else
            symbolIns.append(QString("AC(DEPVS)"));

        if(danger)
            symbolIns.append(QString("SY(ISODGR01)"));
    }

    return symbolIns;
}
void ConditionalItem::setup_DEPVAL02_area(const std::vector<AreaObject>::iterator &object, const int &expsou, const int &watlev, float &leastDepth, float &seabedDepth) const
{
    {
        //后面再处理

    }

    if(leastDepth != -1000) {   //获得leastDepth的值
        if((watlev == 3) && (expsou == 1 || expsou == 3))
            seabedDepth = leastDepth;
        else {
            seabedDepth = leastDepth;
            leastDepth = -1000;
        }
    }
}

bool ConditionalItem::setup_UDWHAZ04_area(std::vector<AreaObject>::iterator &object, const float &depthValue, const int &watlev) const
{
    bool danger = false;
    bool second_danger = false;
    if(depthValue <= MarinerSelect.SAFETY_CONTOUR) {
        //查找每个跟本物标位置交叉或者包含关系的DEPARE和DRGARE物标，并取其DRAVAL1属性进行比较
        {  //检测每个物标,得到是否为危险物标
            QPainterPath object_path;   //物标区域
            {
                std::vector<std::vector<int > >::iterator cit;
                std::vector<std::vector<int > >::iterator cend = object->contours.end();
                for (cit = object->contours.begin(); cit != cend; ++cit) {
                    std::vector<int>::iterator vit;
                    std::vector<int>::iterator vend = cit->end();
                    vit = cit->begin();
                    object_path.moveTo(QPointF(nowChart->sg2ds[*vit].long_lat[0].minutes, nowChart->sg2ds[*vit].long_lat[1].minutes));
                    for (vit = cit->begin()+1; vit != vend; ++vit) {
                        object_path.lineTo(QPointF(nowChart->sg2ds[*vit].long_lat[0].minutes, nowChart->sg2ds[*vit].long_lat[1].minutes));
                    }
                }
            }

            std::map<int, std::vector<AreaObject> >::iterator aom_it;
            std::map<int, std::vector<AreaObject> >::iterator aom_end = nowChart->area_objects_map.end();
            for(aom_it = nowChart->area_objects_map.begin(); aom_it != aom_end; ++aom_it) {
                if(aom_it->first == DEPARE || aom_it->first == DRGARE) {
                    std::vector<AreaObject>::iterator ao_it;
                    std::vector<AreaObject>::iterator ao_end = aom_it->second.end();
                    for(ao_it = aom_it->second.begin(); ao_it != ao_end; ++ao_it) {
                        QPainterPath path;
                        std::vector<std::vector<int > >::iterator cit;
                        std::vector<std::vector<int > >::iterator cend = ao_it->contours.end();
                        for (cit = ao_it->contours.begin(); cit != cend; ++cit) {
                            std::vector<int>::iterator vit;
                            std::vector<int>::iterator vend = cit->end();
                            vit = cit->begin();
                            path.moveTo(QPointF(nowChart->sg2ds[*vit].long_lat[0].minutes, nowChart->sg2ds[*vit].long_lat[1].minutes));
                            for (vit = cit->begin()+1; vit != vend; ++vit) {
                                path.lineTo(QPointF(nowChart->sg2ds[*vit].long_lat[0].minutes, nowChart->sg2ds[*vit].long_lat[1].minutes));
                            }
                        }
                        //检测该区域是否包含该物标点,假设只有一个面包含？
                        if(path.contains(object_path) || path.intersects(object_path)) {
                            float drval1 = -1000;
                            std::vector<attf_t>::iterator attf_it;
                            std::vector<attf_t>::iterator attf_end = ao_it->attfs.end();
                            for(attf_it = ao_it->attfs.begin(); attf_it != attf_end; ++attf_it) {
                                if(attf_it->attl == 87) {  //DRVAL1
                                    drval1 = QString(attf_it->atvl.c_str()).toFloat();
                                    break;
                                }
                            }
                            if(drval1 >= MarinerSelect.SAFETY_CONTOUR ) {
                                danger = true;
                                break;
                            }else if(drval1 >= 0) {
                                second_danger = true;
                                break;
                            }

                        }
                    }  //end areaobject

                }
                if(danger || second_danger)
                    break;
            }

        }
        if(danger) {
            if((watlev == 1) || (watlev == 2)) {
                object->dispcategory = DISPLAY_BASE;   //显示组为14050
                return false;
            }
            object->dispcategory = DISPLAY_BASE;  //显示组为14010
            object->priority = 8;
            object->overRadar = true;
            return true;
        }else {
            if(MarinerSelect.SHOW_ISOLATEDDANGE) {
                //condition A///////////////循环检测与物标有关系的DEPARE DRGARE,确定是否在安全线与深度0线之间
                if(second_danger) {
                    if((watlev == 1) || (watlev == 2)) {
                        object->dispcategory = STANDARD;   //24050
                        return false;
                    }
                    object->dispcategory = STANDARD;  //24020
                    object->priority = 8;
                    object->overRadar = true;
                    return true;
                }
            }else
                return false;
        }
    }else
        return false;
}


QStringList ConditionalItem::setup_QUAPNT02_area(const std::vector<AreaObject>::iterator &object) const
{
    QStringList  symbolIns;
    //循环检测每个引用物标是否有quapos属性
    std::vector<fspt_t>::iterator fit;
    std::vector<fspt_t>::iterator fend = object->fspts.end();
    for(fit = object->fspts.begin(); fit != fend; ++fit) {
        const int ref_record = fit->rcid;
        int quapos = -1;
        std::map<int, EdgeVector>::iterator evm_it;
        evm_it = nowChart->edge_vectors_map.find(ref_record);
        const int size = evm_it->second.attvs.size();
        for(int i=0; i<size; i++) {
            if(evm_it->second.attvs[i].attl == 402) {
                quapos = QString(evm_it->second.attvs[i].atvl.c_str()).toInt();
            }
        }
        if(quapos != -1) {  //没有该属性
            if((quapos > 1) && (quapos < 10)) {  //LC(LOWACC41)绘制该条线段
                symbolIns.append(QString("LC(LOWACC01)"));
                break;
            }
        }
    }

    return symbolIns;
}
QStringList ConditionalItem::setup_OBSTRN04_area(std::vector<AreaObject>::iterator &object) const
{
    QStringList symbolIns;
    //获取属性VALSOU
    float valsou = -1000;
    int expsou = -1;
    int watlev = -1;
    int catobs = -1;

    float depthValue = -1000;
    float leastDepth = -1000;
    QStringList depthSymbol;

    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = object->attfs.end();
    for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
        if(attf_it->attl == 42) {  //CATBOS
            catobs = QString(attf_it->atvl.c_str()).toInt();
        }
        if(attf_it->attl == 93) {  //EXPSOU
            expsou = QString(attf_it->atvl.c_str()).toInt();
        }
        if(attf_it->attl == 179) {   //VALSOU
            valsou = QString(attf_it->atvl.c_str()).toFloat();
        }
        if(attf_it->attl == 187) {   //WATLEV
            watlev = QString(attf_it->atvl.c_str()).toInt();
        }
     }


    //线物标应该没有valsou属性
    if((valsou != -1000) && (valsou != 0.0)) {   //将viewgroup变为34051,没用这个,调用SNDFRM03(depthValue)条件程序,即显示该深度值出来
        int quapos = -1;   //位置精度
        depthValue = valsou;
        depthSymbol = setup_SNDFRM03(depthValue, quapos, (object->attfs));
    }else {
         //调用DEPVAL02条件程序,以这两个属性值为参数,返回leastDepth,seabedDepth
        float seabedDepth;
         setup_DEPVAL02_area(object, expsou, watlev, leastDepth, seabedDepth);
        if(leastDepth == -1000) {
            if(catobs == 6)
                depthValue = 0.01;
            else if(watlev == 5)
                depthValue = 0;
            else if(watlev == 3)
                depthValue = 0.01;
            else
                depthValue = -15;
        }else
            depthValue = leastDepth;
    }

    bool danger = setup_UDWHAZ04_area(object, depthValue, watlev);
    symbolIns.append(setup_QUAPNT02_area(object));
    if(danger) {
        symbolIns.append(QString("AC(DEPVS)"));
        symbolIns.append(QString("AP(FOULAR01)"));
        symbolIns.append(QString("LS(DOTT,2,CHBLK)"));
        symbolIns.append(QString("SY(ISODGR01)"));
        return symbolIns;
    }
    if((valsou != -1000) && (valsou != 0.0)) {
        if(valsou <= 20)
            symbolIns.append(QString("LS(DOTT,2,CHBLK)"));
        else
            symbolIns.append(QString("LS(DASH,2,CHGRD)"));
        for(int i=0; i < depthSymbol.size(); i++) {
            symbolIns.append(QString("SY(%1)").arg(depthSymbol.at(i)));   //返回的是不带SY标志的符号，需要加上
        }
    }else {
        if(catobs == 6) {
            symbolIns.append(QString("AP(FOULAR01)"));
            symbolIns.append(QString("LS(DOTT,2,CHBLK)"));
        }else if(watlev == 1 || watlev == 2) {
            symbolIns.append(QString("AC(CHBRN)"));
            symbolIns.append(QString("LS(SOLD,2,CSTLN)"));
        }else if(watlev == 4) {
            symbolIns.append(QString("AC(DEPIT)"));
            symbolIns.append(QString("LS(DASH,2,CSTLN)"));
        }else {
            symbolIns.append(QString("AC(DEPVS)"));
            symbolIns.append(QString("LS(DOTT,2,CHBLK)"));
        }
    }

    return symbolIns;
}
//*********************************线型条件物标处理********************************
QStringList ConditionalItem::setup_DATCVR01(const std::vector<LineObject>::iterator &object) const
{
    //描述的不是太清除，后面有时间再弄
    QStringList symbolIns;
    return symbolIns;

}
QStringList ConditionalItem::setup_DEPCNT02(const std::vector<LineObject>::iterator &object) const
{
    QStringList symbolIns;
    //循环检测，是否含有属性QUAPOS,此处不再检测该属性，线段默认不包含，需要的话要从连接节点的属性中查找并存储进来,直接显示LS(SOLD,1,DEPCN)
    std::vector<fspt_t>::iterator fit;
    std::vector<fspt_t>::iterator fend = object->fspts.end();
    for(fit = object->fspts.begin(); fit != fend; ++fit) {
        int ref_record = fit->rcid;
        //检测有没有QUAPOS属性,一般都没有,可以去掉
        int quapos = 0;
        std::map<int, EdgeVector>::iterator evm_it;
        evm_it = nowChart->edge_vectors_map.find(ref_record);
        const int size = evm_it->second.attvs.size();
        for(int i=0; i < size; i++) {
            if(evm_it->second.attvs[i].attl == 402) {
                quapos = QString(evm_it->second.attvs[i].atvl.c_str()).toInt();
            }
        }
        if((quapos > 1) && (quapos < 10))
            symbolIns.append(QString("LS(DASH,1,DEPCN,%1,)").arg(ref_record));   //自定义的格式
        else
            symbolIns.append(QString("LS(SOLD,1,DEPCN,%1,)").arg(ref_record));

/*        //默认显示等高线标志,检测属性VALDCO
        float Loc_Valdco = 0.0;
        std::vector<attf_t>::iterator attf_it;
        std::vector<attf_t>::iterator attf_end = object->attfs.end();
        for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
            if((attf_it->attl == 174)) {   //VALDCO
                const QString at = QString(attf_it->atvl.c_str());   //物标属性值
                Loc_Valdco = at.toFloat();
            }
        }
        //调用SAFCON01条件程序，得到一系列符号名称
        //得到的符号显示在线物标中心，显示的图形不需要于轮廓线对齐，需要垂直显示
*/
    }
    return symbolIns;
}

QStringList ConditionalItem::setup_QUAPOS01_Line(const std::vector<LineObject>::iterator &object) const
{
    QStringList symbolIns;
    //调用QUALIN01条件程序，直接写在这里,需要使用空间物标的属性
    //如果属性QUAPOS的值为23456789,则使用LC(LOWACC21)，不需要继续下面的处理，该属性暂时不处理，
    std::vector<fspt_t>::iterator fit;
    std::vector<fspt_t>::iterator fend = object->fspts.end();
    for(fit = object->fspts.begin(); fit != fend; ++fit) {
        int ref_record = fit->rcid;
        //检测有没有QUAPOS属性,一般都没有,可以去掉
        int quapos = 0;
        std::map<int, EdgeVector>::iterator evm_it;
        evm_it = nowChart->edge_vectors_map.find(ref_record);
        const int size = evm_it->second.attvs.size();
        for(int i=0; i < size; i++) {
            if(evm_it->second.attvs[i].attl == 402) {
                quapos = QString(evm_it->second.attvs[i].atvl.c_str()).toInt();
            }
        }
        if((quapos > 1) && (quapos < 10)) {
            symbolIns.append(QString("LC(LOWACC21,%1,)").arg(ref_record));   //自定义的格式
            continue;
        }

        if(object->object_label == COALNE) {
            bool flag = false;
            std::vector<attf_t>::iterator attf_it;
            std::vector<attf_t>::iterator attf_end = object->attfs.end();
            for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
                if((attf_it->attl == 82)) {   //CONRAD
                    const QString at = QString(attf_it->atvl.c_str());   //物标属性值
                    if(at == "1") {   //LS(SOLD,3,CHMGF)  LS(SOLD,1,CSTLN)
                        symbolIns.append(QString("LS(SOLD,3,CHMGF,%1,)").arg(ref_record));
                        symbolIns.append(QString("LS(SOLD,1,CSTLN,%1,)").arg(ref_record));
                    }else{  //LS(SOLD,1,CSTLN)
                        symbolIns.append(QString("LS(SOLD,1,CSTLN,%1,)").arg(ref_record));
                    }
                    flag = true;
                    break;
                }
            }
            if(!flag) {   //LS(SOLD,1,CSTLN)
                symbolIns.append(QString("LS(SOLD,1,CSTLN,%1,)").arg(ref_record));
            }
        }else {  //LNDARE  ，LS(SOLD,1,CSTLN)
            symbolIns.append(QString("LS(SOLD,1,CSTLN,%1,)").arg(ref_record));
        }
    }
    return symbolIns;

}
QStringList ConditionalItem::setup_OBSTRN04_line(std::vector<LineObject>::iterator &object) const
{
    QStringList symbolIns;
    //获取属性VALSOU
    float valsou = -1000;
    int expsou = -1;
    int watlev = -1;
    int catobs = -1;

    float depthValue = -1000;
    float leastDepth = -1000;
    QStringList depthSymbol;

    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = object->attfs.end();
    for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
        if(attf_it->attl == 42) {  //CATBOS
            catobs = QString(attf_it->atvl.c_str()).toInt();
        }
        if(attf_it->attl == 93) {  //EXPSOU
            expsou = QString(attf_it->atvl.c_str()).toInt();
        }
        if(attf_it->attl == 179) {   //VALSOU
            valsou = QString(attf_it->atvl.c_str()).toFloat();
        }
        if(attf_it->attl == 187) {   //WATLEV
            watlev = QString(attf_it->atvl.c_str()).toInt();
        }
     }


    //线物标应该没有valsou属性
    if((valsou != -1000) && (valsou != 0.0)) {   //将viewgroup变为34051,没用这个,调用SNDFRM03(depthValue)条件程序,即显示该深度值出来
        int quapos = -1;   //位置精度
        depthValue = valsou;
        depthSymbol = setup_SNDFRM03(depthValue, quapos, (object->attfs));
    }else {
         //调用DEPVAL02条件程序,以这两个属性值为参数,返回leastDepth,seabedDepth
        float seabedDepth;
         setup_DEPVAL02_line(object, expsou, watlev, leastDepth, seabedDepth);
        if(leastDepth == -1000) {
            if(catobs == 6)
                depthValue = 0.01;
            else if(watlev == 5)
                depthValue = 0;
            else if(watlev == 3)
                depthValue = 0.01;
            else
                depthValue = -15;
        }else
            depthValue = leastDepth;
    }

   // bool danger = setup_UDWHAZ04_point(object, depthValue, watlev);
    bool danger = false;    //UDWHAZ04资料上写的不包括Line类型，所以设定为没有独立危险标志

    std::vector<fspt_t>::iterator fit;
    std::vector<fspt_t>::iterator fend = object->fspts.end();
    for(fit = object->fspts.begin(); fit != fend; ++fit) {
        int ref_record = fit->rcid;
        int quapos = -1;
        std::map<int, EdgeVector>::iterator evm_it;
        evm_it = nowChart->edge_vectors_map.find(ref_record);
        const int size = evm_it->second.attvs.size();
        for(int i=0; i<size; i++) {
            if(evm_it->second.attvs[i].attl == 402) {
                quapos = QString(evm_it->second.attvs[i].atvl.c_str()).toInt();
            }
        }
        if(quapos != -1) {  //有该属性
            if((quapos > 1) && (quapos < 10)) {  //LC(LOWACC41)绘制该条线段
                if(danger)
                    symbolIns.append(QString("LC(LOWACC41,%1,)").arg(ref_record));
                else
                    symbolIns.append(QString("LC(LOWACC31,%1,)").arg(ref_record));
                continue;
            }
        }

        if(danger) {
            symbolIns.append(QString("LS(DOTT,2,CHBLK,%1,)").arg(ref_record));
            continue;
        }

        if((valsou != -1000) && (valsou != 0.0)) {
            if(valsou <= 20)
                symbolIns.append(QString("LS(DOTT,2,CHBLK,%1,)").arg(ref_record));
            else
                symbolIns.append(QString("LS(DASH,2,CHBLK,%1,)").arg(ref_record));
        }else
            symbolIns.append(QString("LS(DOTT,2,CHBLK,%1,)").arg(ref_record));

     }

    if(danger)
        symbolIns.append(QString("SY(ISODRG01)"));  //显示在线段的中心
    else {
        if((valsou != -1000) && (valsou != 0.0))
            for(int i=0; i < depthSymbol.size(); i++) {
                symbolIns.append(QString("SY(%1)").arg(depthSymbol.at(i)));   //返回的是不带SY标志的符号，需要加上
            }
    }

    return symbolIns;
}
void ConditionalItem::setup_DEPVAL02_line(const std::vector<LineObject>::iterator &object, const int &expsou, const int &watlev, float &leastDepth, float &seabedDepth) const
{
    {
        //后面再处理

    }

    if(leastDepth != -1000) {   //获得leastDepth的值
        if((watlev == 3) && (expsou == 1 || expsou == 3))
            seabedDepth = leastDepth;
        else {
            seabedDepth = leastDepth;
            leastDepth = -1000;
        }
    }
}
QStringList ConditionalItem::setup_SAFCON01(const float &depthValue) const
{
    QStringList symbolIns;
    QString prefix = QString("SAFCON");

    if(depthValue < 0 || depthValue > 99999)
        return symbolIns;
    if((depthValue < 10) && ((depthValue - (int)depthValue) != 0)) {
        int value = (int)depthValue;
        QString name = prefix + QString("0") + QString::number(value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)((depthValue - (int)depthValue) * 10);
        name = prefix + QString::number(60 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        return symbolIns;
    }
    if(depthValue < 10) {
        QString name = prefix + QString("0") + QString::number((int)depthValue);
        symbolIns.append(QString("SY(%1)").arg(name));
        return symbolIns;
    }
    if((depthValue < 31) && ((depthValue - (int)depthValue) != 0)) {
        int value = (int)depthValue / 10;
        QString name =  prefix + QString::number(20 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue % 10;
        name = prefix + QString::number(10 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)((depthValue - (int)depthValue) * 10);
        name = prefix + QString::number(50 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        return symbolIns;
    }
    if(depthValue < 100) {
        int value = (int)depthValue / 10;
        QString name =  prefix + QString::number(20 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue % 10;
        name = prefix + QString::number(10 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        return symbolIns;
    }
    if(depthValue < 1000) {
        int value = (int)depthValue / 100;
        QString name =  prefix + QString::number(80 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue / 10 % 10;
        name = prefix + QString("0") + QString::number(value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue % 10;
        name =  prefix + QString::number(90 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        return symbolIns;
    }
    if(depthValue < 10000) {
        int value = (int)depthValue / 1000;
        QString name =  prefix + QString::number(30 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue /100 % 10;
        name = prefix + QString::number(20 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue /10 % 10;
        name = prefix + QString::number(10 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue % 10;
        name = prefix + QString::number(70 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        return symbolIns;
    }
    if(depthValue < 100000) {
        int value = (int)depthValue / 10000;
        QString name =  prefix + QString::number(40 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue /1000 % 10;
        name = prefix + QString::number(30 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue /100 % 10;
        name = prefix + QString::number(20 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue /10 % 10;
        name = prefix + QString::number(10 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        value = (int)depthValue % 10;
        name = prefix + QString::number(70 + value);
        symbolIns.append(QString("SY(%1)").arg(name));
        return symbolIns;
    }

}
//*********************************点条件物标处理********************************
QStringList ConditionalItem::setup_DATCVR01(const std::vector<PointObject>::iterator &object) const
{
    //描述的不是太清除，后面有时间再弄
    QStringList symbolIns;
    return symbolIns;

}
QStringList ConditionalItem::setup_QUAPOS01_Point(const std::vector<PointObject>::iterator &object) const
{
    //调用QUAPNT02条件程序，该程序也被WRECKS物标使用,不再单独作为函数
    //如果系统选择显示LOW ACCURACY SYMBOLS
    QStringList symbolIns;
    int quapos = 0;
    std::vector<attv_t>::iterator attv_it;
    std::vector<attv_t>::iterator attv_end = object->attvs.end();
    for(attv_it = object->attvs.begin(); attv_it != attv_end; ++ attv_it){
        if((attv_it->attl == 402)) {
            quapos = QString(attv_it->atvl.c_str()).toInt();
        }
    }
    if((quapos > 1) && (quapos < 10)) {
        symbolIns.append(QString("SY(LOWACC01)"));

    }

    return symbolIns;
}
//灯光物标的显示，有可能有问题
QStringList ConditionalItem::setup_LIGHTS05(const std::vector<PointObject>::iterator &object) const
{
    QStringList symbolIns;
    //获取属性VALNMR, CATLIT    
    QStringList catlit;
    QStringList colour;
    QStringList litvis;
    float valnmr = 9;
    float orient = -1000;
    float sectr1 = -1000;
    float sectr2 = -1000;


    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = object->attfs.end();
    for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
        switch(attf_it->attl) {
            case 178:  //VALNMR
                valnmr = QString(attf_it->atvl.c_str()).toFloat();
            break;
            case 37:  //CATLIT
                catlit = QString(attf_it->atvl.c_str()).split(",");
            break;
            case 117:    //ORIENT
                orient = QString(attf_it->atvl.c_str()).toFloat();
            break;
            case 75:   //COLOUR
               colour = QString(attf_it->atvl.c_str()).split(",");
            break;
            case 136:  //SECTR1
                sectr1 = QString(attf_it->atvl.c_str()).toFloat();
            break;
            case 137:  //SECTR2
                sectr2 = QString(attf_it->atvl.c_str()).toFloat();
            break;
            case 108:  //LITVIS
                litvis = QString(attf_it->atvl.c_str()).split(",");
            break;
        }
    }

    //含有CATLIT属性
    if(! catlit.isEmpty()) {
        if(catlit.contains("8") || catlit.contains("11")) {   //显示符号SY(LIGHTS82)
            symbolIns.append(QString("SY(LIGHTS82)"));
            return symbolIns;
        }else if(catlit.contains("9")) {  //显示符号SY(LIGHTS81)
            symbolIns.append(QString("SY(LIGHTS81)"));
            return symbolIns;
        }else if(catlit.contains("1") || catlit.contains("16")) {
            if(abs(orient) < 360) {  //含有ORIENT属性， LS(DASH,1,CHBLK)，方向大小从朝海方向，看到后面是+/-180度？？？旋转方向可能不对??
                symbolIns.append(QString("LS(DASH,1,CHBLK,%1,%2,)").arg(valnmr*2).arg(orient));  //自定义格式，加的length，方向
            }
        }
    }

    //是否有COLOUR属性
    if(colour.isEmpty())
        colour.append(QString("12"));
    //SECTR1或者SECTR2缺失
    if((sectr1 == -1000) || (sectr2 == -1000)) {
        bool otherNoSecLight = false;
        bool flareAtDegree = false;
        QString lightName;
        //需要检测在该点有没有其他灯物标，以及他们的SECTR属性
        std::map<int, std::vector<PointObject> >::iterator pom_it;
        std::map<int, std::vector<PointObject> >::iterator pom_end = nowChart->point_objects_map.end();
        for(pom_it = nowChart->point_objects_map.begin(); pom_it != pom_end; ++pom_it) {
            if(pom_it->first == LIGHTS) {
                std::vector<PointObject>::iterator po_it;
                std::vector<PointObject>::iterator po_end = pom_it->second.end();
                for (po_it = pom_it->second.begin(); po_it != po_end; ++po_it) {
                    if((po_it->record_id != object->record_id) && (po_it->index == object->index)) {
                        otherNoSecLight = true;
                        std::vector<attf_t>::iterator attf_it;
                        std::vector<attf_t>::iterator attf_end = po_it->attfs.end();
                        for(attf_it = po_it->attfs.begin(); attf_it != attf_end; ++ attf_it){
                            if((attf_it->attl == 136) || (attf_it->attl == 137)) {   //SECTR1/SECTR2
                                otherNoSecLight = false;
                                break;
                            }
                        }
                    }
                    break;  //假设最多一个重合的灯物标
                }
                break;
            }
        }

        if(otherNoSecLight) { //有其他NO SECTR LIGHTS 在同一点
            if(colour.contains("1") || colour.contains("6") || colour.contains("11"))
                flareAtDegree = true;
        }
        //选择合适符号,适当简化
        if(colour.contains("3"))
            lightName = "LIGHTS11";
        else if(colour.contains("4"))
            lightName = "LIGHTS12";
        else if(colour.contains("11") || colour.contains("6") || colour.contains("1"))
            lightName = "LIGHTS13";
        else
            lightName = "LITDEF11";

        if(catlit.contains("1") || catlit.contains("16")) {
             if(abs(orient) < 360) {
                 symbolIns.append(QString("SY(%1,%2,)").arg(lightName).arg(orient));
                 symbolIns.append(QString("TE('%03.01f deg','ORIENT',3,3,3,'15110',3,1,CHBLK,23)"));
             }else
                 symbolIns.append(QString("SY(QUESMRK1)"));
        }else{
            if(flareAtDegree)
                symbolIns.append(QString("SY(%1,%2,)").arg(lightName).arg(45));   //垂直方向旋转45度
            else
                symbolIns.append(QString("SY(%1,%2,)").arg(lightName).arg(135));   //垂直方向旋转135度
        }
        //通过程序LITDSN01得到灯物标的描述语句，但是该程序只在数字发行版中才有，所以没法做
        return symbolIns;

    }else {   //两个扇形角度都有
        QString lightName;
        float leglen;

        if(sectr1 == 0) {   //没有属性值,但是有属性
            sectr1 = 0;
            sectr2 = 0;
        }
        if((sectr2 - sectr1) == 0) {  //圆形灯
            if(colour.contains("3"))
                lightName = "LIGHTS11";
            else if(colour.contains("4"))
                lightName = "LIGHTS12";
            else if(colour.contains("11") || colour.contains("6") || colour.contains("1"))
                lightName = "LIGHTS13";
            else
                lightName = "LITDEF11";
            //文字描述不能实现
            symbolIns.append(QString("SY(%1,%2,)").arg(lightName).arg(135));   //垂直方向旋转135度

            return symbolIns;
        }else {
            if(sectr2 <= sectr1)  sectr2 += 360;
            //航海人员选择显示灯的两根角度线长，是否显示全部长度.        默认显示LS(DASH,1,CHBLK)
            leglen = MarinerSelect.FULL_LIGHTS ? valnmr*2 : 25*2 ;    //25mm, 或者为valnmr,转化为像素，每毫米3.7个像素

            symbolIns.append(QString("LS(DASH,1,CHBLK,%1,%2,)").arg(leglen).arg(sectr1));  //自定义格式，加的length，方向(垂直旋转)
            symbolIns.append(QString("LS(DASH,1,CHBLK,%1,%2,)").arg(leglen).arg(sectr2));
        }

#define compare(x, m, n)   ((m < x) && (x < n))
        //查找是否需要将弧形圈放大，有其他灯物标在上面没，并且计算扇形覆盖
        bool extendArc = false;
        std::map<int, std::vector<PointObject> >::iterator pom_it;
        std::map<int, std::vector<PointObject> >::iterator pom_end = nowChart->point_objects_map.end();
        for(pom_it = nowChart->point_objects_map.begin(); pom_it != pom_end; ++pom_it) {
            if(pom_it->first == LIGHTS) {
                std::vector<PointObject>::iterator po_it;
                std::vector<PointObject>::iterator po_end = pom_it->second.end();
                for (po_it = pom_it->second.begin(); po_it != po_end; ++po_it) {
                    if((po_it->record_id != object->record_id) && (po_it->index == object->index)) {
                        //检测每个相同位置灯物标的扇形宽度
                        float sec1,sec2;
                        std::vector<attf_t>::iterator attf_it;
                        std::vector<attf_t>::iterator attf_end = po_it->attfs.end();
                        for(attf_it = po_it->attfs.begin(); attf_it != attf_end; ++ attf_it){
                            if((attf_it->attl == 136)) {   //SECTR1
                                sec1 = QString(attf_it->atvl.c_str()).toFloat();
                            }
                            if((attf_it->attl == 137)) {  //SECTR2
                                sec2 = QString(attf_it->atvl.c_str()).toFloat();
                                break;
                            }
                        }
                        if(compare(sec1,sectr1,sectr2) || compare(sec2,sectr1,sectr2) || compare(sectr1,sec1,sec2) || compare(sectr2,sec1,sec2)) {
                            if((sec2 - sec1) > (sectr2 - sectr1))
                                extendArc = true;
                        }
                    }
                }
                break;
            }
        }

        if(litvis.contains("7") || litvis.contains("8") || litvis.contains("3")) {   //LS(DASH,1,CHBLK)
            if(extendArc)   //半径25mm
                symbolIns.append(QString("AR(DASH,1,CHBLK,%1,%2,%3,)").arg(25*2).arg(sectr1).arg(sectr2));  //自定义,后面加半径，扇形两个角度
            else    //半径20mm
                symbolIns.append(QString("AR(DASH,1,CHBLK,%1,%2,%3,)").arg(20*2).arg(sectr1).arg(sectr2));
        }else{
            QString color;
            if(colour.contains("3"))
                color = "LITRD";
            else if(colour.contains("4"))
                color = "LITGN";
            else if(colour.contains("11") || colour.contains("6") || colour.contains("1"))
                color = "LITYW";
            else
                color = "CHMGD";
            if(extendArc) {             //LS(SOLD,2,color)
                //显绘LS(SOLD,4,OUTLW),再绘上面的,25mm,具体为多少像素不知道
                symbolIns.append(QString("AR(SOLD,4,OUTLW,%1,%2,%3,)").arg(25*2).arg(sectr1).arg(sectr2));
                symbolIns.append(QString("AR(SOLD,2,%1,%2,%3,%4,)").arg(color).arg(25*2).arg(sectr1).arg(sectr2));

            }else {  //20mm
                symbolIns.append(QString("AR(SOLD,4,OUTLW,%1,%2,%3,)").arg(20*2).arg(sectr1).arg(sectr2));
                symbolIns.append(QString("AR(SOLD,2,%1,%2,%3,%4,)").arg(color).arg(20*2).arg(sectr1).arg(sectr2));
            }

        }
        return symbolIns;
    }
}

QStringList ConditionalItem::setup_WRECKS02_point(std::vector<PointObject>::iterator &object) const
{
    QStringList symbolIns;
    float depthValue = -1000;
    float leastDepth = -1000;
    float seabedDepth = -1000;
    float valsou = -1000;
    int catwrk = -1;
    int watlev = -1;
    int expsou = -1;
    QStringList symbol;

    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = object->attfs.end();
    for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
        if(attf_it->attl == 71) {   //CATWRK
            catwrk = QString(attf_it->atvl.c_str()).toInt();  continue;
        }
        if(attf_it->attl == 93) {  //EXPSOU
            expsou = QString(attf_it->atvl.c_str()).toInt();  continue;
        }
        if(attf_it->attl == 179) {  //VALSOU
            const QString at = QString(attf_it->atvl.c_str());              
            valsou = at.toFloat();
            continue;
        }
        if(attf_it->attl == 187) {   //WATLEV
            watlev = QString(attf_it->atvl.c_str()).toInt();   continue;
        }      
    }
    if((valsou != -1000) && (valsou != 0.0)) {   //将viewgroup变为34051,没用这个,调用SNDFRM03(depthValue)条件程序,即显示该深度值出来
        int quapos = -1;   //位置精度
        std::vector<attv_t>::iterator attv_it;
        std::vector<attv_t>::iterator attv_end = object->attvs.end();
        for(attv_it = object->attvs.begin(); attv_it != attv_end; ++attv_it) {
            if((attv_it->attl == 402)) {   //QUAPOS
                quapos = QString(attv_it->atvl.c_str()).toInt();
            }
        }
        depthValue = valsou;
        symbol = setup_SNDFRM03(depthValue, quapos, (object->attfs));
    }else {   //没有给出VALSOU属性值
         //调用DEPVAL02条件程序,以这两个属性值为参数,返回leastDepth,seabedDepth
         setup_DEPVAL02_point(object, expsou, watlev, leastDepth, seabedDepth);
         if(leastDepth == -1000) {
             if(catwrk > 0) {  //有CATWRK属性
                 if(catwrk == 1) {
                     depthValue = 20.1;
                     if(seabedDepth != -1000) {
                         leastDepth = seabedDepth - 66.0;
                         if(! (leastDepth < 20.1)) {
                             depthValue = leastDepth;
                         }
                     }
                 }else
                     depthValue = -15;
             }else{
                 if(watlev > 0) {
                     if((watlev == 3) || (watlev == 5))
                         depthValue = 0;
                     else
                         depthValue = -15;
                 }else
                     depthValue = -15;
             }
         }else
             depthValue = leastDepth;         
     }

     //为点物标
     //调用UDWHAZ04('depthValue'),得到是否显示isolated danger符号,SY(ISODGR01)
     bool danger = setup_UDWHAZ04_point(object, depthValue, watlev);
   //  danger = false;
     symbolIns.append(setup_QUAPOS01_Point(object));      //调用QUAPNT02条件程序得到是否显示LOW ACCURACY符号
     if(danger) {
         symbolIns.append(QString("SY(ISODGR01)"));         
         return symbolIns;
     }else {
         //condition A//////////////////////////////
         if(valsou > 0) {
             if(valsou <= 20)
                 symbolIns.append(QString("SY(DANGER01)"));
             else
                 symbolIns.append(QString("SY(DANGER02)"));
             for(int i=0; i < symbol.size(); i++) {
                 symbolIns.append(QString("SY(%1)").arg(symbol.at(i)));   //返回的是不带SY标志的符号，需要加上
             }

         }else {
             QString name;
             if((catwrk == 1) && (watlev == 3))
                 name = "WRECKS04";
             else if((catwrk == 2) && (watlev == 3))
                 name = "WRECKS05";
             else if((catwrk == 4) || (catwrk == 5) || (watlev == 1)|| (watlev == 2)|| (watlev == 5)|| (watlev == 4))
                 name = "WRECKS01";
             else
                 name = "WRECKS05";

             symbolIns.append(QString("SY(%1)").arg(name));
         }
         return symbolIns;
     }
}

bool ConditionalItem::setup_UDWHAZ04_point(std::vector<PointObject>::iterator &object, const float &depthValue, const int &watlev) const
{
    bool danger = false;
    bool second_danger = false;   //用于二次检测标记，一次检测完成
    if(depthValue <= MarinerSelect.SAFETY_CONTOUR) {
        //查找每个跟本物标位置交叉或者包含关系的DEPARE和DRGARE物标，并取其DRAVAL1属性进行比较,利用QPainterPath的接口来处理，后面可以自己写写
        {  //检测每个物标,得到是否为危险物标
            std::map<int, std::vector<AreaObject> >::iterator aom_it;
            std::map<int, std::vector<AreaObject> >::iterator aom_end = nowChart->area_objects_map.end();
            for(aom_it = nowChart->area_objects_map.begin(); aom_it != aom_end; ++aom_it) {
                if(aom_it->first == DEPARE || aom_it->first == DRGARE) {
                    std::vector<AreaObject>::iterator ao_it;
                    std::vector<AreaObject>::iterator ao_end = aom_it->second.end();
                    for(ao_it = aom_it->second.begin(); ao_it != ao_end; ++ao_it) {
                        QPainterPath path;
                        std::vector<std::vector<int > >::iterator cit;
                        std::vector<std::vector<int > >::iterator cend = ao_it->contours.end();
                        for (cit = ao_it->contours.begin(); cit != cend; ++cit) {
                            std::vector<int>::iterator vit;
                            std::vector<int>::iterator vend = cit->end();
                            vit = cit->begin();
                            path.moveTo(QPointF(nowChart->sg2ds[*vit].long_lat[0].minutes, nowChart->sg2ds[*vit].long_lat[1].minutes));
                            for (vit = cit->begin()+1; vit != vend; ++vit) {
                                path.lineTo(QPointF(nowChart->sg2ds[*vit].long_lat[0].minutes, nowChart->sg2ds[*vit].long_lat[1].minutes));
                            }
                        }
                        //检测该区域是否包含该物标点,假设只有一个面包含？
                        if(path.contains(QPointF(nowChart->sg2ds[object->index].long_lat[0].minutes, nowChart->sg2ds[object->index].long_lat[1].minutes))) {
                            float drval1 = -1000;
                            std::vector<attf_t>::iterator attf_it;
                            std::vector<attf_t>::iterator attf_end = ao_it->attfs.end();
                            for(attf_it = ao_it->attfs.begin(); attf_it != attf_end; ++attf_it) {
                                if(attf_it->attl == 87) {  //DRVAL1
                                    drval1 = QString(attf_it->atvl.c_str()).toFloat();
                                    break;
                                }
                            }
                            if(drval1 >= MarinerSelect.SAFETY_CONTOUR ) {
                                danger = true;
                                break;
                            }else if(drval1 >= 0) {
                                second_danger = true;
                                break;
                            }

                        }
                    }  //end areaobject

                }
                if(danger || second_danger)
                    break;
            }

        }
        if(danger) {
            if((watlev == 1) || (watlev == 2)) {
                object->dispcategory = DISPLAY_BASE;   //显示组为14050
                return false;
            }
            object->dispcategory = DISPLAY_BASE;  //显示组为14010
            object->priority = 8;
            object->overRadar = true;
            return true;
        }else {
            if(MarinerSelect.SHOW_ISOLATEDDANGE) {
                //condition A///////////////循环检测与物标有关系的DEPARE DRGARE,确定是否在安全线与深度0线之间,由second_danger代替

                if(second_danger) {
                    if((watlev == 1) || (watlev == 2)) {
                        object->dispcategory = STANDARD;   //24050
                        return false;
                    }
                    //默认不显示DANGER IN SHALLOW
                    /*object->dispcategory = STANDARD;  //24020
                    object->priority = 8;
                    object->overRadar = true;
                    return true; */
                    return false;
                }else
                    return false;
            }else
                return false;
        }
    }else
        return false;
}

void ConditionalItem::setup_DEPVAL02_point(const std::vector<PointObject>::iterator &object, const int &expsou, const int &watlev, float &leastDepth, float &seabedDepth) const
{
    //循环检测调用该程序的物标所覆盖（或者相交）的每个物标，是否为UNSARE， DEPARE， DRGARE
    //需要的数据处理： 点是否在面物标中
    //面物标与面物标是否交叉或者包含,      通过这些面物标的DRVAL1属性得到leastDepth的值
    {
        bool flag = false;
        std::map<int, std::vector<AreaObject> >::iterator aom_it;
        std::map<int, std::vector<AreaObject> >::iterator aom_end = nowChart->area_objects_map.end();
        for(aom_it = nowChart->area_objects_map.begin(); aom_it != aom_end; ++aom_it) {
            if(aom_it->first == UNSARE) {
                std::vector<AreaObject>::iterator ao_it;
                std::vector<AreaObject>::iterator ao_end = aom_it->second.end();
                for(ao_it = aom_it->second.begin(); ao_it != ao_end; ++ao_it) {
                    QPainterPath path;
                    std::vector<std::vector<int > >::iterator cit;
                    std::vector<std::vector<int > >::iterator cend = ao_it->contours.end();
                    for (cit = ao_it->contours.begin(); cit != cend; ++cit) {
                        std::vector<int>::iterator vit;
                        std::vector<int>::iterator vend = cit->end();
                        vit = cit->begin();
                        path.moveTo(QPointF(nowChart->sg2ds[*vit].long_lat[0].minutes, nowChart->sg2ds[*vit].long_lat[1].minutes));
                        for (vit = cit->begin()+1; vit != vend; ++vit) {
                            path.lineTo(QPointF(nowChart->sg2ds[*vit].long_lat[0].minutes, nowChart->sg2ds[*vit].long_lat[1].minutes));
                        }
                    }
                    //检测该区域是否包含该物标点
                    if(path.contains(QPointF(nowChart->sg2ds[object->index].long_lat[0].minutes, nowChart->sg2ds[object->index].long_lat[1].minutes))) {
                        leastDepth = -1000;
                        seabedDepth = -1000;
                        flag = true;
                        break;
                    }
                }  //end areaobject
            }else if(aom_it->first == DEPARE || aom_it->first == DRGARE) {
                std::vector<AreaObject>::iterator ao_it;
                std::vector<AreaObject>::iterator ao_end = aom_it->second.end();
                for(ao_it = aom_it->second.begin(); ao_it != ao_end; ++ao_it) {
                    QPainterPath path;
                    std::vector<std::vector<int > >::iterator cit;
                    std::vector<std::vector<int > >::iterator cend = ao_it->contours.end();
                    for (cit = ao_it->contours.begin(); cit != cend; ++cit) {
                        std::vector<int>::iterator vit;
                        std::vector<int>::iterator vend = cit->end();
                        vit = cit->begin();
                        path.moveTo(QPointF(nowChart->sg2ds[*vit].long_lat[0].minutes, nowChart->sg2ds[*vit].long_lat[1].minutes));
                        for (vit = cit->begin()+1; vit != vend; ++vit) {
                            path.lineTo(QPointF(nowChart->sg2ds[*vit].long_lat[0].minutes, nowChart->sg2ds[*vit].long_lat[1].minutes));
                        }
                    }
                    //检测该区域是否包含该物标点,假设只有一个面包含？
                    if(path.contains(QPointF(nowChart->sg2ds[object->index].long_lat[0].minutes, nowChart->sg2ds[object->index].long_lat[1].minutes))) {
                        float drval1 = -1000;
                        std::vector<attf_t>::iterator attf_it;
                        std::vector<attf_t>::iterator attf_end = ao_it->attfs.end();
                        for(attf_it = ao_it->attfs.begin(); attf_it != attf_end; ++attf_it) {
                            if(attf_it->attl == 87) {  //DRVAL1
                                drval1 = QString(attf_it->atvl.c_str()).toFloat();
                                break;
                            }
                        }
                        if((drval1 != 1000) && (drval1 != 0)) {
                            if((leastDepth == -1000) || (leastDepth > drval1))
                                leastDepth = drval1;

                        }
                        break;
                        flag = true;
                    }
                }  //end areaobject

            }
            if(flag)
                break;
        }

    }

    if(leastDepth != -1000) {   //获得leastDepth的值
        if((watlev == 3) && (expsou == 1 || expsou == 3))
            seabedDepth = leastDepth;
        else {
            seabedDepth = leastDepth;
            leastDepth = -1000;
        }
    }
}

//显示深度数字的程序
QStringList ConditionalItem::setup_SNDFRM03(float depthValue, const int &quapos, std::vector<attf_t> attfs) const
{
    QStringList symbolName;
    QString symbolPrefix;
    QStringList quasou,status;

    if(depthValue <= MarinerSelect.SAFETY_CONTOUR)
        symbolPrefix = QString("SOUNDS");
    else
        symbolPrefix = QString("SOUNDG");

    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = attfs.end();
    for(attf_it = attfs.begin(); attf_it != attf_end; ++ attf_it){
        if(attf_it->attl == 156) {  //TECSOU
            const QStringList at = QString(attf_it->atvl.c_str()).split(",");
            if(at.contains("6")) {
                QString name = symbolPrefix + QString("B1");
                symbolName.append(name);
            }
        }
        if(attf_it->attl == 125) {  //QUASOU
            quasou = QString(attf_it->atvl.c_str()).split(",");
        }
        if(attf_it->attl == 149) {  //STATUS
            status = QString(attf_it->atvl.c_str()).split(",");
        }
     }

    if(status.contains("18") || quasou.contains("3") || quasou.contains("4") || quasou.contains("5") || quasou.contains("8") || quasou.contains("9")) {
        QString name = symbolPrefix + QString("C2");
        symbolName.append(name);
    }else {
        if((quapos > 1) && (quapos < 10)) {
                QString name = symbolPrefix + QString("C2");
                symbolName.append(name);
        }
    }

    ////////////////////////condition A////////////////////
    if(depthValue < 0) {
        QString name = symbolPrefix + QString("A1");
        depthValue -= 0.02;   //在处理的时候加上的，负数要补回来
        depthValue = depthValue < 0 ? -depthValue : depthValue;   //负数变正数
        symbolName.append(name);
    }
    if(depthValue < 10) {      
        const int depth = (int)depthValue + 10;
        const int decimal = (int)((depthValue - (int)depthValue) * 10) + 50;
        QString value = QString::number(depth);
        QString name = symbolPrefix + value;
        symbolName.append(name);
        value = QString::number(decimal);
        name = symbolPrefix + value;
        symbolName.append(name);

        return symbolName;
    }else if((depthValue < 31) && ((float)(depthValue - (int)depthValue) != 0)) {
        const int d1 = ((int)depthValue) / 10;
        const int d2 = ((int)depthValue) % 10;
        const int d3 = (int)((depthValue - (int)depthValue) * 10);
        QString value = QString::number(d1 + 20);
        QString name = symbolPrefix + value;
        symbolName.append(name);
        value = QString::number(d2 + 10);
        name = symbolPrefix + value;
        symbolName.append(name);
        value = QString::number(d3 + 50);
        name = symbolPrefix + value;
        symbolName.append(name);

        return symbolName;
    }else if((int)depthValue < 100) {
        const int d1 = (int)depthValue / 10;
        const int d2 = (int)depthValue % 10;
        QString value = QString::number(d1 + 10);
        QString name = symbolPrefix + value;
        symbolName.append(name);
        value = QString("0") + QString::number(d2);
        name = symbolPrefix + value;
        symbolName.append(name);

        return symbolName;
    }else if((int)depthValue < 1000) {
        const int d1 = (int)depthValue / 100;
        const int d2 = (int)depthValue % 100 / 10;
        const int d3 = (int)depthValue % 10;
        QString value = QString::number(d1 + 20);
        QString name = symbolPrefix + value;
        symbolName.append(name);
        value = QString::number(d2 + 10);
        name = symbolPrefix + value;
        symbolName.append(name);
        value = QString("0") + QString::number(d3);
        name = symbolPrefix + value;
        symbolName.append(name);

        return symbolName;
    }else if((int)depthValue < 10000) {
        const int d1 = (int)depthValue / 1000;
        const int d2 = (int)depthValue % 1000 / 100;
        const int d3 = (int)depthValue % 100 / 10;
        const int d4 = (int)depthValue % 10;
        QString value = QString::number(d1 + 20);
        QString name = symbolPrefix + value;
        symbolName.append(name);
        value = QString::number(d2 + 10);
        name = symbolPrefix + value;
        symbolName.append(name);
        value = QString("0") + QString::number(d3);
        name = symbolPrefix + value;
        symbolName.append(name);
        value = QString::number(d4 + 40);
        name = symbolPrefix + value;
        symbolName.append(name);

        return symbolName;
    }else {
        const int d1 = (int)depthValue / 10000;
        const int d2 = (int)depthValue % 10000 / 1000;
        const int d3 = (int)depthValue % 1000 / 100;
        const int d4 = (int)depthValue % 100 / 10;
        const int d5 = (int)depthValue % 10;
        QString value = QString::number(d1 + 30);
        QString name = symbolPrefix + value;
        symbolName.append(name);
        value = QString::number(d2 + 20);
        name = symbolPrefix + value;
        symbolName.append(name);
        value = QString::number(d3 + 10);
        name = symbolPrefix + value;
        symbolName.append(name);
        value = QString("0") + QString::number(d4);
        name = symbolPrefix + value;
        symbolName.append(name);
        value = QString::number(d5 + 40);
        name = symbolPrefix + value;
        symbolName.append(name);

        return symbolName;
    }


}

QStringList ConditionalItem::setup_SOUNDG02(const std::vector<SoundObject>::iterator &object) const
{
    //对于每个深度点，调用SNDFRM03得到显示的符号并显示即可,该函数不使用，直接在绘制那边处理
    QStringList symbolIns;
    int indexs = 0;
    std::vector<sg3d_t>::iterator it;
    std::vector<sg3d_t>::iterator end = object->sg3ds.end();
    for(it = object->sg3ds.begin(); it != end; ++it) {
        const float depthValue = it->depth;
        int quapos = 0;
        std::vector<attv_t>::iterator attv_it;
        std::vector<attv_t>::iterator attv_end = object->attvs.end();
        for(attv_it = object->attvs.begin(); attv_it != attv_end; ++ attv_it){
            if((attv_it->attl == 402)) {
                quapos = QString(attv_it->atvl.c_str()).toInt();
            }
        }
        QStringList symbol = setup_SNDFRM03(depthValue, quapos, object->attfs);
        const int size  = symbol.size();
        for(int i=0; i<size; i++) {
            symbolIns.append(QString("SY(%1,%2)").arg(symbol.at(i).arg(indexs)));  //自定义，前面表示名字，后面位置标号
        }
        indexs++;
    }
    return symbolIns;
}

QStringList ConditionalItem::setup_TOPMAR01(const std::vector<PointObject>::iterator &object) const
{
    QStringList symbolIns;
    //获取属性TOPSHP
    int topshp = -1;
    QString symbolName;

    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = object->attfs.end();
    for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
        if(attf_it->attl == 171) {   //TOPSHP
            topshp = QString(attf_it->atvl.c_str()).toInt();
            break;
        }
     }

    if(topshp == -1) {  //显示QUESMRK1
        symbolIns.append(QString("SY(QUESMRK1)"));

    }else {
        //查找在该点的其他物标LITFLT LITVES BOY...  BCN...
        bool floating = false;
        bool find = false;
        std::map<int, std::vector<PointObject> >::iterator pom_it = nowChart->point_objects_map.begin();
        std::map<int, std::vector<PointObject> >::iterator pom_end = nowChart->point_objects_map.end();
        for(; pom_it != pom_end; ++pom_it) {
            switch(pom_it->first) {
            case LITFLT:
            case LITVES:
            case BOYCAR:
            case BOYINB:
            case BOYISD:
            case BOYLAT:
            case BOYSAW:
            case BOYSPP:
            {
                std::vector<PointObject>::iterator po_it = pom_it->second.begin();
                std::vector<PointObject>::iterator po_end = pom_it->second.end();
                for(; po_it != po_end; ++po_it) {
                    if(po_it->index == object->index){
                        floating = true;
                        find = true;
                    }
                }
            }
            break;
            case BCNCAR:
            case BCNISD:
            case BCNLAT:
            case BCNSAW:
            case BCNSPP:
            {
                std::vector<PointObject>::iterator po_it = pom_it->second.begin();
                std::vector<PointObject>::iterator po_end = pom_it->second.end();
                for(; po_it != po_end; ++po_it) {
                    if(po_it->index == object->index){
                        floating = false;
                        find = true;
                    }
                }
            }
            break;
            }
            if(find)
                break;
        }

        if(floating) {
            switch(topshp) {
                case 1:
                case 24:
                case 29: symbolName = "TOPMAR02";   break;
                case 2:
                case 25:  symbolName = "TOPMAR04";   break;
                case 3:
                case 18:
                case 26:
                case 32:  symbolName = "TOPMAR10";   break;
                case 4: symbolName = "TOPMAR12";   break;
                case 5:
                case 19:
                case 21:  symbolName = "TOPMAR13";   break;
                case 6:
                case 12:
                case 20:
                case 22:
                case 23:
                case 31:  symbolName = "TOPMAR14";   break;
                case 13:    symbolName = "TOPMAR05";   break;
                case 14:    symbolName = "TOPMAR06";   break;
                case 27:
                case 30:    symbolName = "TOPMAR17";   break;
                case 28:    symbolName = "TOPMAR18";   break;
                default:  symbolName = "TMARDEF2";   break;
            }
        }else {
            switch(topshp) {
                case 1:  case 24: case 29:    symbolName = "TOPMAR22";   break;
                case 2:  case 25:  symbolName = "TOPMAR24";   break;
                case 3:  case 18: case 26: case 32:  symbolName = "TOPMAR30";   break;
                case 4: symbolName = "TOPMAR32";   break;
                case 5:  case 19: case 21:  symbolName = "TOPMAR33";   break;
                case 6:  case 20: case 22:  case 23:   symbolName = "TOPMAR34";   break;
                case 7:    symbolName = "TOPMAR85";   break;
                case 9:    symbolName = "TOPMAR36";   break;
                case 10:    symbolName = "TOPMAR28";   break;
                case 11:    symbolName = "TOPMAR27";   break;
                case 12:  case 31:   symbolName = "TOPMAR14";   break;
                case 13:    symbolName = "TOPMAR25";   break;
                case 14:    symbolName = "TOPMAR26";   break;
                case 15:    symbolName = "TOPMAR88";   break;
                case 16:    symbolName = "TOPMAR87";   break;
                case 8: case 27:  case 30:    symbolName = "TOPMAR86";   break;
                case 28:    symbolName = "TOPMAR89";   break;
                default:  symbolName = "TMARDEF1";   break;
            }
        }
        symbolIns.append(QString("SY(%1)").arg(symbolName));
    }
    return symbolIns;
}

QStringList ConditionalItem::setup_OBSTRN04_point(std::vector<PointObject>::iterator &object) const
{
    QStringList symbolIns;
    //获取属性VALSOU
    float valsou = -1000;
    int expsou = -1;
    int watlev = -1;
    int catobs = -1;

    float depthValue = -1000;
    float leastDepth = -1000;
    QStringList depthSymbol;

    std::vector<attf_t>::iterator attf_it;
    std::vector<attf_t>::iterator attf_end = object->attfs.end();
    for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
        if(attf_it->attl == 42) {  //CATBOS
            catobs = QString(attf_it->atvl.c_str()).toInt();
        }
        if(attf_it->attl == 93) {  //EXPSOU
            expsou = QString(attf_it->atvl.c_str()).toInt();
        }
        if(attf_it->attl == 179) {   //VALSOU
            valsou = QString(attf_it->atvl.c_str()).toFloat();
        }
        if(attf_it->attl == 187) {   //WATLEV
            watlev = QString(attf_it->atvl.c_str()).toInt();
        }
     }


    if((valsou != -1000) && (valsou != 0.0)) {   //将viewgroup变为34051,没用这个,调用SNDFRM03(depthValue)条件程序,即显示该深度值出来
        int quapos = -1;   //位置精度
        std::vector<attv_t>::iterator attv_it;
        std::vector<attv_t>::iterator attv_end = object->attvs.end();
        for(attv_it = object->attvs.begin(); attv_it != attv_end; ++attv_it) {
            if((attv_it->attl == 402)) {   //QUAPOS
                quapos = QString(attv_it->atvl.c_str()).toInt();
            }
        }
        depthValue = valsou;
        depthSymbol = setup_SNDFRM03(depthValue, quapos, (object->attfs));
    }else {
         //调用DEPVAL02条件程序,以这两个属性值为参数,返回leastDepth,seabedDepth
        float seabedDepth;
         setup_DEPVAL02_point(object, expsou, watlev, leastDepth, seabedDepth);
        if(leastDepth == -1000) {
            if(catobs == 6)
                depthValue = 0.01;
            else if(watlev == 5)
                depthValue = 0;
            else if(watlev == 3)
                depthValue = 0.01;
            else
                depthValue = -15;
        }else
            depthValue = leastDepth;       
    }

    bool danger = setup_UDWHAZ04_point(object, depthValue, watlev);
    symbolIns.append(setup_QUAPOS01_Point(object));      //调用QUAPNT02条件程序得到是否显示LOW ACCURACY符号
    if(danger) {
        symbolIns.append(QString("SY(ISODGR01)"));
        return symbolIns;
    }

    bool sounding = false;
    QString symbolName;
    if(valsou > 0) {
        if(valsou <= 20) {
            if(object->object_label == UWTROC) {  //为类型UWTROC
                if(watlev == 4 || watlev == 5)  {
                    symbolName = "UWTROC04";
                    sounding = false;
                }else{
                    symbolName = "DANGER01";
                    sounding = true;
                }

            }else{  //类型OBSTRN
                if(catobs == 6){
                    symbolName = "DANGER01";
                    sounding = true;
                }else if(watlev == 1 || watlev == 2){
                    symbolName = "OBSTRN11";
                    sounding = false;
                }else if(watlev == 3){
                    symbolName = "DANGER01";
                    sounding = true;
                }else if(watlev == 4 || watlev == 5){
                    symbolName = "DANGER03";
                    sounding = true;
                }else{
                    symbolName = "DANGER01";
                    sounding = true;
                }
            }

        }else {
            symbolName = "DANGER02";
            sounding = true;
        }
    }else{
        if(object->object_label == UWTROC) { //为类型UWTROC
            if(watlev == 3){
                symbolName = "UWTROC03";
            }else
                symbolName = "UWTROC04";
        }else {
            if(catobs == 6){
                symbolName = "OBSTRN01";
            }else if(watlev == 1 || watlev == 2){
                symbolName = "OBSTRN11";
            }else if(watlev == 3){
                symbolName = "OBSTRN01";
            }else if(watlev == 4 || watlev == 5){
                symbolName = "OBSTRN03";
            }else{
                symbolName = "OBSTRN01";
            }
        }
    }
    symbolIns.append(QString("SY(%1)").arg(symbolName));
    if(sounding) {
        for(int i=0; i < depthSymbol.size(); i++) {
            symbolIns.append(QString("SY(%1)").arg(depthSymbol.at(i)));   //返回的是不带SY标志的符号，需要加上
        }
    }
    return symbolIns;
}







void ConditionalItem::setupConditionalNameMap()
{
    conditional_name_num_map.insert("CLRLIN01", CLRLIN01);
    conditional_name_num_map.insert("DATCVR01", DATCVR01);
    conditional_name_num_map.insert("DEPARE01", DEPARE01);
    conditional_name_num_map.insert("DEPCNT02", DEPCNT02);
    conditional_name_num_map.insert("DEPVAL02", DEPVAL02);
    conditional_name_num_map.insert("LEGLIN03", LEGLIN03);
    conditional_name_num_map.insert("LIGHTS05", LIGHTS05);
    conditional_name_num_map.insert("LITDSN01", LITDSN01);
    conditional_name_num_map.insert("OBSTRN04", OBSTRN04);
    conditional_name_num_map.insert("OWNSHP02", OWNSHP02);
    conditional_name_num_map.insert("PASTRK01", PASTRK01);
    conditional_name_num_map.insert("QUAPOS01", QUAPOS01);
    conditional_name_num_map.insert("QUALIN01", QUALIN01);
    conditional_name_num_map.insert("QUAPNT02", QUAPNT02);
    conditional_name_num_map.insert("RESARE02", RESARE02);
    conditional_name_num_map.insert("RESTRN01", RESTRN01);
    conditional_name_num_map.insert("RESCSP02", RESCSP02);
    conditional_name_num_map.insert("SAFCON01", SAFCON01);
    conditional_name_num_map.insert("SLCONS03", SLCONS03);
    conditional_name_num_map.insert("SEABED01", SEABED01);
    conditional_name_num_map.insert("SNDFRM03", SNDFRM03);
    conditional_name_num_map.insert("SOUNDG02", SOUNDG02);
    conditional_name_num_map.insert("SYMINSnn", SYMINSnn);
    conditional_name_num_map.insert("TOPMAR01", TOPMAR01);
    conditional_name_num_map.insert("UDWHAZ04", UDWHAZ04);
    conditional_name_num_map.insert("VESSEL02", VESSEL02);
    conditional_name_num_map.insert("VRMEBL02", VRMEBL02);
    conditional_name_num_map.insert("WRECKS02", WRECKS02);

}

