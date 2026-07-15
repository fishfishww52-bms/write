"""
run_all_mem.py — 内存模式：
    跑算法 → 内存生成图表 → 浏览器打开 → 关了回收内存
    不存 CSV，不存 HTML 历史

用法:
    python run_all_mem.py <csv_name>
    python run_all_mem.py 12_10_1
"""
import re
import sys
import json
import tempfile
import subprocess
import webbrowser
import pandas as pd
from io import StringIO
from pathlib import Path

ROOT = Path(__file__).parent
SOC_SIM = ROOT / "build" / "soc_sim.exe"
MS100 = ROOT.parent / "data" / "excel" / "ms100"


def parse_cap(filename: str) -> int:
    m = re.match(r"(\d+)_", filename)
    return int(m.group(1)) if m else 38


def build_tree(columns):
    groups = {}
    for col in columns:
        if col == "tick":
            continue
        parts = col.split(".")
        if len(parts) >= 2:
            grp, display = parts[0], col[len(parts[0])+1:]
        else:
            grp, display = "_other", col
        groups.setdefault(grp, []).append({"col": col, "display": display})
    return [{"name": g, "children": sorted(f, key=lambda x: x["display"])}
            for g, f in sorted(groups.items())]


def run_sim(csv_path: Path) -> pd.DataFrame:
    df_in = pd.read_csv(csv_path)
    cap = parse_cap(csv_path.stem)

    lines = []
    for i, row in df_in.iterrows():
        v = int(row["voltage"])
        c = int(row["current"])
        if i == 0:
            lines.append(f"{v},{c},{cap},0,25,{v}")
        else:
            lines.append(f"{v},{c}")

    result = subprocess.run(
        [str(SOC_SIM)],
        input="\n".join(lines),
        capture_output=True, text=True, timeout=300,
    )
    if result.returncode != 0:
        raise RuntimeError(f"算法异常: {result.stderr[-300:]}")

    return pd.read_csv(StringIO(result.stdout))


def generate_html(df: pd.DataFrame, stem: str) -> str:
    columns = list(df.columns)
    tree = build_tree(columns)
    cd = {c: df[c].tolist() for c in columns}

    # 采样（>50K点缩到1/10）
    step = 10 if len(df) > 50000 else 1
    cd_sampled = {c: v[::step] for c, v in cd.items()}

    html = f"""<!DOCTYPE html>
<html lang="zh"><head><meta charset="utf-8"><title>{stem}</title>
<script src="https://cdn.plot.ly/plotly-3.0.1.min.js"></script>
<style>
*{{margin:0;padding:0;box-sizing:border-box}}
body{{font-family:'Segoe UI',sans-serif;display:flex;height:100vh;width:100vw;overflow:hidden}}
#sidebar{{width:280px;min-width:280px;overflow-y:auto;background:#f5f5f5;border-right:1px solid #ccc;padding:10px}}
#sidebar h3{{font-size:13px;margin-bottom:4px}}
.grp{{margin-bottom:2px}}
.grp-name{{cursor:pointer;font-weight:600;font-size:12px;padding:2px 6px;border-radius:3px;user-select:none}}
.grp-name:hover{{background:#ddd}}
.grp-name::before{{content:'+ ';font-size:11px;font-weight:bold}}
.grp-name.open::before{{content:'- '}}
.grp-children{{display:none;margin-left:14px}}
.grp-children.open{{display:block}}
.grp-children label{{display:block;font-size:11px;padding:1px 4px;cursor:pointer;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}}
.grp-children label:hover{{background:#e0e0e0}}
.grp-children input{{margin-right:4px}}
#toolbar{{margin-bottom:6px;display:flex;align-items:center;gap:8px}}
#toolbar button{{padding:2px 10px;font-size:11px;cursor:pointer;border:1px solid #aaa;border-radius:3px;background:#fff}}
#toolbar button:hover{{background:#eee}}
#toolbar span{{font-size:11px;color:#888}}
#plot-area{{flex:1;overflow-y:auto;overflow-x:hidden;padding:4px}}
.chart-box{{border:1px solid #e0e0e0;border-radius:3px;margin-bottom:2px;background:#fff}}
.chart-box:last-child{{margin-bottom:0}}
.chart-title{{font-size:9px;color:#888;padding:1px 6px;background:#fafafa;border-bottom:1px solid #eee;height:15px;line-height:13px}}
</style></head><body>
<div id="sidebar">
<h3>信号选择</h3>
<div id="toolbar"><button onclick="clearAll()">清空全部</button><span id="info"></span></div>
<div id="tree"></div>
</div>
<div id="plot-area"></div>
<script>
var TI={json.dumps(cd_sampled['tick'])};
var CD={json.dumps(cd_sampled)};
var TR={json.dumps(tree)};
var ST={step};
var selected=[],plotIds=[],yRanges=[],clearTimer=null,lastX=null;
var COLORS=['#1f77b4','#ff7f0e','#2ca02c','#d62728','#9467bd','#8c564b','#e377c2','#7f7f7f','#bcbd22','#17becf'];

function buildTree(){{var c=document.getElementById('tree');
for(var gi=0;gi<TR.length;gi++){{var g=TR[gi];var d=document.createElement('div');d.className='grp';
var nm=document.createElement('div');nm.className='grp-name';nm.textContent=g.name+' ('+g.children.length+')';
var ch=document.createElement('div');ch.className='grp-children';
nm.onclick=(function(n,c){{return function(){{n.classList.toggle('open');c.classList.toggle('open')}}}})(nm,ch);
for(var fi=0;fi<g.children.length;fi++){{var f=g.children[fi];var lb=document.createElement('label');
var cb=document.createElement('input');cb.type='checkbox';
cb.onchange=(function(col,bx){{return function(){{onToggle(col,bx.checked)}}}})(f.col,cb);
lb.appendChild(cb);lb.appendChild(document.createTextNode(f.display));ch.appendChild(lb);}}
d.appendChild(nm);d.appendChild(ch);c.appendChild(d);}}}}

function getChartH(n){{var p=18;return Math.max(70,Math.min(320,Math.floor((window.innerHeight-12)/n)-p));}}
var syncLock=false;
function syncZoom(x0,x1){{if(syncLock)return;syncLock=true;
for(var j=0;j<plotIds.length;j++)Plotly.relayout(plotIds[j],{{'xaxis.range':[x0,x1]}});
setTimeout(function(){{syncLock=false}},250);}}
function syncReset(){{if(syncLock)return;syncLock=true;
for(var j=0;j<plotIds.length;j++)Plotly.relayout(plotIds[j],{{'xaxis.autorange':true}});
setTimeout(function(){{syncLock=false}},250);}}

function rebuild(){{var n=selected.length;
document.getElementById('info').textContent=n+' signal'+(n!==1?'s':'');
var area=document.getElementById('plot-area');area.innerHTML='';
for(var j=0;j<plotIds.length;j++){{try{{Plotly.purge(plotIds[j])}}catch(e){{}}}}
plotIds=[];yRanges=[];lastX=null;if(n===0)return;
var h=getChartH(n);
for(var i=0;i<n;i++){{var col=selected[i];
var label=col.indexOf('.')>=0?col.slice(col.indexOf('.')+1):col;
var box=document.createElement('div');box.className='chart-box';
var tt=document.createElement('div');tt.className='chart-title';tt.textContent=col;
box.appendChild(tt);var pd=document.createElement('div');var pid='c'+i;
pd.id=pid;pd.style.width='100%';pd.style.height=h+'px';
box.appendChild(pd);area.appendChild(box);plotIds.push(pid);
var y=CD[col];if(!y)continue;
yRanges.push({{min:Math.min.apply(null,y),max:Math.max.apply(null,y)}});
var clr=COLORS[i%COLORS.length];
var trace={{x:TI,y:y,mode:'lines',name:label,line:{{color:clr,width:1.2}},hoverinfo:'none'}};
var layout={{margin:{{l:48,r:8,t:2,b:16}},hovermode:'x',showlegend:false,dragmode:'zoom',
xaxis:{{title:'tick'}},yaxis:{{title:label,zeroline:true,fixedrange:true}},automargin:false}};
Plotly.newPlot(pid,[trace],layout,{{responsive:true,displayModeBar:false}});
(function(p){{var el=document.getElementById(p);
el.on('plotly_relayout',function(ed){{if(ed['xaxis.autorange']!==undefined)syncReset();
else{{var x0=ed['xaxis.range[0]'],x1=ed['xaxis.range[1]'];if(x0!==undefined)syncZoom(x0,x1);}}}});
el.on('plotly_hover',function(ev){{if(clearTimer){{clearTimeout(clearTimer);clearTimer=null;}}
var x=Math.round(ev.xvals[0]/ST);if(x!==lastX){{lastX=x;drawCursors(x);}}}});
el.on('plotly_unhover',function(){{scheduleClear();}});}})(pid);
}}}}

function drawCursors(x){{if(x<0||x>=TI.length)return;
var mx=TI[TI.length-1];
for(var j=0;j<plotIds.length;j++){{var col=selected[j],y=CD[col][x];if(y===undefined)continue;
var label=col.indexOf('.')>=0?col.slice(col.indexOf('.')+1):col,clr=COLORS[j%COLORS.length];
var ann={{x:x,y:y,text:label+'='+y,showarrow:false,font:{{size:10,color:'#fff'}},bgcolor:clr,borderpad:2}};
if(x>mx*0.5){{ann.xanchor='right';ann.xshift=-2;}}else{{ann.xanchor='left';ann.xshift=2;}}
var rng=yRanges[j];if(y>(rng.min+rng.max)/2){{ann.yanchor='top';ann.yshift=-2;}}else{{ann.yanchor='bottom';ann.yshift=2;}}
Plotly.relayout(plotIds[j],{{shapes:[{{type:'line',x0:x,x1:x,y0:0,y1:1,yref:'paper',
line:{{color:clr,width:1.5,dash:'dot'}}}}],annotations:[ann]}});}}}}

function scheduleClear(){{if(clearTimer)clearTimeout(clearTimer);
clearTimer=setTimeout(function(){{for(var j=0;j<plotIds.length;j++)Plotly.relayout(plotIds[j],{{shapes:[],annotations:[]}});
lastX=null;clearTimer=null;}},120);}}
function onToggle(col,checked){{if(checked){{if(selected.indexOf(col)===-1)selected.push(col);}}
else{{var idx=selected.indexOf(col);if(idx!==-1)selected.splice(idx,1);}}rebuild();}}
function clearAll(){{document.querySelectorAll('#sidebar input[type=checkbox]').forEach(function(cb){{cb.checked=false}});
selected=[];rebuild();}}
window.onresize=function(){{if(selected.length)rebuild();}};
buildTree();
</script></body></html>"""
    return html


def main():
    if len(sys.argv) < 2:
        print(f"用法: python {sys.argv[0]} <csv_name>")
        print(f"例:   python {sys.argv[0]} 12_10_1")
        sys.exit(1)

    csv_path = MS100 / f"{sys.argv[1]}.csv"
    if not csv_path.exists():
        print(f"找不到: {csv_path}")
        sys.exit(1)

    # 1. 跑算法 → 内存
    print(f"运行算法...")
    df = run_sim(csv_path)
    print(f"  {len(df)} 行 × {len(df.columns)} 列")

    # 2. 生成 HTML → 临时文件
    print(f"生成图表...")
    html = generate_html(df, csv_path.stem)

    tmp = tempfile.NamedTemporaryFile(suffix=".html", mode="w", encoding="utf-8", delete=False)
    tmp.write(html)
    tmp.close()

    # 3. 打开浏览器
    webbrowser.open(tmp.name)
    print(f"  {csv_path.stem} — 已打开")
    print(f"  关闭浏览器后，临时文件自动回收")
    # Windows 下没法直接等到浏览器关闭，但 tell user


if __name__ == "__main__":
    main()
