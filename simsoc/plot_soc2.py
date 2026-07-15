"""
plot_soc2.py — SOC 交互式绘图
独立图框 + 跨图游标 + 缩放同步 + 无滚动条
"""
import sys
import json
import pandas as pd
from pathlib import Path


def build_tree(columns):
    groups = {}
    for col in columns:
        if col == "tick":
            continue
        parts = col.split(".")
        if len(parts) >= 2:
            grp = parts[0]
            display = col[len(grp)+1:]
        else:
            grp = "_other"
            display = col
        groups.setdefault(grp, []).append({"col": col, "display": display})
    return [{"name": g, "children": sorted(f, key=lambda x: x["display"])}
            for g, f in sorted(groups.items())]


def generate_html(csv_path: Path):
    df = pd.read_csv(csv_path)
    cols = [c for c in df.columns]
    tree = build_tree(cols)
    cd = {c: df[c].tolist() for c in cols}
    stem = csv_path.stem
    out_path = csv_path.with_suffix(".html")

    # 减小数据量：每10个点取1个（45K -> 4.5K，肉眼看不出区别）
    cd_sampled = {}
    step = 10
    for c in cols:
        arr = cd[c]
        cd_sampled[c] = arr[::step] if len(arr) > 10000 else arr

    html = """<!DOCTYPE html>
<html lang="zh">
<head><meta charset="utf-8"><title>__STEM__</title>
<script src="https://cdn.plot.ly/plotly-3.0.1.min.js"></script>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',sans-serif;display:flex;height:100vh;width:100vw;overflow:hidden}
#sidebar{width:280px;min-width:280px;overflow-y:auto;background:#f5f5f5;border-right:1px solid #ccc;padding:10px}
#sidebar h3{font-size:13px;margin-bottom:4px}
.grp{margin-bottom:2px}
.grp-name{cursor:pointer;font-weight:600;font-size:12px;padding:2px 6px;border-radius:3px;user-select:none}
.grp-name:hover{background:#ddd}
.grp-name::before{content:'+ ';font-size:11px;font-weight:bold}
.grp-name.open::before{content:'- '}
.grp-children{display:none;margin-left:14px}
.grp-children.open{display:block}
.grp-children label{display:block;font-size:11px;padding:1px 4px;cursor:pointer;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.grp-children label:hover{background:#e0e0e0}
.grp-children input{margin-right:4px}
#toolbar{margin-bottom:6px;display:flex;align-items:center;gap:8px}
#toolbar button{padding:2px 10px;font-size:11px;cursor:pointer;border:1px solid #aaa;border-radius:3px;background:#fff}
#toolbar button:hover{background:#eee}
#toolbar span{font-size:11px;color:#888}
#plot-area{flex:1;overflow-y:auto;overflow-x:hidden;padding:4px}
.chart-box{border:1px solid #e0e0e0;border-radius:3px;margin-bottom:2px;background:#fff}
.chart-box:last-child{margin-bottom:0}
.chart-title{font-size:9px;color:#888;padding:1px 6px;background:#fafafa;border-bottom:1px solid #eee;height:15px;line-height:13px}
</style></head>
<body>
<div id="sidebar">
<h3>信号选择</h3>
<div id="toolbar"><button onclick="clearAll()">清空全部</button><span id="info"></span></div>
<div id="tree"></div>
</div>
<div id="plot-area"></div>
<script>
var TIME=__TICK__;
var COLDATA=__COLDATA__;
var TREE=__TREE__;
var selected=[];
var plotIds=[];
var yRanges=[]; // {min,max} per selected signal
var clearTimer=null;
var lastX=null;
var STEP=__STEP__;
var COLORS=['#1f77b4','#ff7f0e','#2ca02c','#d62728','#9467bd','#8c564b','#e377c2','#7f7f7f','#bcbd22','#17becf'];

function buildTree(){
    var c=document.getElementById('tree');
    for(var gi=0;gi<TREE.length;gi++){
        var grp=TREE[gi];
        var div=document.createElement('div');div.className='grp';
        var nm=document.createElement('div');nm.className='grp-name';
        nm.textContent=grp.name+' ('+grp.children.length+')';
        var ch=document.createElement('div');ch.className='grp-children';
        nm.onclick=(function(n,c){return function(){n.classList.toggle('open');c.classList.toggle('open')}})(nm,ch);
        for(var fi=0;fi<grp.children.length;fi++){
            var f=grp.children[fi];
            var lb=document.createElement('label');
            var cb=document.createElement('input');cb.type='checkbox';
            cb.onchange=(function(col,bx){return function(){onToggle(col,bx.checked)}})(f.col,cb);
            lb.appendChild(cb);
            lb.appendChild(document.createTextNode(f.display));
            ch.appendChild(lb);
        }
        div.appendChild(nm);div.appendChild(ch);c.appendChild(div);
    }
}

function getChartH(n){
    var per=18; // title+border+margin per chart
    return Math.max(70, Math.min(320, Math.floor((window.innerHeight-12)/n)-per));
}

var syncLock=false;
function syncZoom(x0,x1){
    if(syncLock)return;syncLock=true;
    for(var j=0;j<plotIds.length;j++){
        Plotly.relayout(plotIds[j],{'xaxis.range':[x0,x1]});
    }
    setTimeout(function(){syncLock=false;},250);
}
function syncReset(){
    if(syncLock)return;syncLock=true;
    for(var j=0;j<plotIds.length;j++){
        Plotly.relayout(plotIds[j],{'xaxis.autorange':true});
    }
    setTimeout(function(){syncLock=false;},250);
}

function rebuild(){
    var n=selected.length;
    document.getElementById('info').textContent=n+' signal'+(n!==1?'s':'');
    var area=document.getElementById('plot-area');
    area.innerHTML='';
    for(var j=0;j<plotIds.length;j++){try{Plotly.purge(plotIds[j])}catch(e){}}
    plotIds=[];yRanges=[];lastX=null;
    if(n===0)return;

    var h=getChartH(n);

    for(var i=0;i<n;i++){
        var col=selected[i];
        var label=col.indexOf('.')>=0?col.slice(col.indexOf('.')+1):col;
        var box=document.createElement('div');box.className='chart-box';
        var tt=document.createElement('div');tt.className='chart-title';tt.textContent=col;
        box.appendChild(tt);
        var pd=document.createElement('div');
        var pid='c'+i;
        pd.id=pid;pd.style.width='100%';pd.style.height=h+'px';
        box.appendChild(pd);area.appendChild(box);
        plotIds.push(pid);

        var y=COLDATA[col];
        if(!y){console.error('no data for',col);continue;}
        var ymax=Math.max.apply(null,y), ymin=Math.min.apply(null,y);
        yRanges.push({min:ymin, max:ymax});
        var clr=COLORS[i%COLORS.length];
        var trace={x:TIME, y:y, mode:'lines', name:label,
            line:{color:clr,width:1.2}, hoverinfo:'none'};
        var layout={margin:{l:48,r:8,t:2,b:16},hovermode:'x',
            showlegend:false,dragmode:'zoom',
            xaxis:{title:'tick'},yaxis:{title:label,zeroline:true,fixedrange:true},
            automargin:false};

        Plotly.newPlot(pid,[trace],layout,{responsive:true,displayModeBar:false});

        // 缩放同步
        (function(p){
            var el=document.getElementById(p);
            el.on('plotly_relayout',function(ed){
                if(ed['xaxis.autorange']!==undefined)syncReset();
                else{var x0=ed['xaxis.range[0]'],x1=ed['xaxis.range[1]'];
                if(x0!==undefined)syncZoom(x0,x1);}
            });
            el.on('plotly_hover',function(ev){
                if(clearTimer){clearTimeout(clearTimer);clearTimer=null;}
                var x=Math.round(ev.xvals[0]);
                if(x!==lastX){lastX=x;drawCursors(x);}
            });
            el.on('plotly_unhover',function(){scheduleClear();});
        })(pid);
    }
}

function drawCursors(x){
    var idx=Math.round(x/STEP);
    if(idx<0||idx>=TIME.length)return;
    var maxTick=TIME[TIME.length-1];
    for(var j=0;j<plotIds.length;j++){
        var col=selected[j];
        var y=COLDATA[col][idx];
        if(y===undefined)continue;
        var label=col.indexOf('.')>=0?col.slice(col.indexOf('.')+1):col;
        var clr=COLORS[j%COLORS.length];
        // 右侧20%区域标签放左边，其他放右边
        var isRight=(x>maxTick*0.5);
        var ann={x:x,y:y,text:label+'='+y,showarrow:false,
            font:{size:10,color:'#fff'},bgcolor:clr,borderpad:2};
        if(isRight){ann.xanchor='right';ann.xshift=-2;}
        else{ann.xanchor='left';ann.xshift=2;}
        var rng=yRanges[j];
        if(y>(rng.min+rng.max)/2){ann.yanchor='top';ann.yshift=-2;}
        else{ann.yanchor='bottom';ann.yshift=2;}
        Plotly.relayout(plotIds[j],{
            shapes:[{type:'line',x0:x,x1:x,y0:0,y1:1,yref:'paper',line:{color:clr,width:1.5,dash:'dot'}}],
            annotations:[ann]
        });
    }
}

function scheduleClear(){
    if(clearTimer)clearTimeout(clearTimer);
    clearTimer=setTimeout(function(){
        for(var j=0;j<plotIds.length;j++){
            try{Plotly.relayout(plotIds[j],{shapes:[],annotations:[]})}catch(e){}
        }
        lastX=null;clearTimer=null;
    },120);
}

function onToggle(col,checked){
    if(checked){
        if(selected.indexOf(col)===-1)selected.push(col);
    }else{
        var idx=selected.indexOf(col);
        if(idx!==-1)selected.splice(idx,1);
    }
    rebuild();
}

function clearAll(){
    var cbs=document.querySelectorAll('#sidebar input[type=checkbox]');
    for(var i=0;i<cbs.length;i++)cbs[i].checked=false;
    selected=[];rebuild();
}
window.onresize=function(){if(selected.length)rebuild();};
if(!window.Plotly){document.body.innerHTML='<h3>Plotly.js 加载失败，检查网络连接</h3>'}
else{buildTree();}
</script></body></html>"""

    html = (html.replace("__STEM__", stem)
            .replace("__STEP__", str(step))
            .replace("__TICK__", json.dumps(cd_sampled["tick"]))
            .replace("__COLDATA__", json.dumps(cd_sampled))
            .replace("__TREE__", json.dumps(tree)))

    out_path.write_text(html, encoding="utf-8")
    print(f"OK {out_path}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"用法: python {sys.argv[0]} <soc_result.csv>")
        sys.exit(1)
    generate_html(Path(sys.argv[1]))
