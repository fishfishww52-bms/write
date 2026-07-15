"""
run_all.py — 一键完整流程

用法:
    python run_all.py <csv_name>             # 跑单个文件（不含 .csv 后缀）
    python run_all.py 12_10_1                # 只跑 data/ms100/12_10_1.csv
    python run_all.py all                    # 跑 ms100 下所有 CSV

流程:
  ms100/<name>.csv → soc_sim.exe → <name>_soc_result.csv → <name>_soc_result.html
"""
import sys
import subprocess
from pathlib import Path

ROOT = Path(__file__).parent
SOC_SIM = ROOT / "build" / "soc_sim.exe"
MS100 = ROOT.parent / "data" / "excel" / "ms100"


def run_one(csv_path: Path):
    name = csv_path.stem
    result_csv = csv_path.parent / f"{name}_soc_result.csv"
    result_html = csv_path.parent / f"{name}_soc_result.html"

    print(f"\n{'='*60}")
    print(f"处理: {name}")
    print(f"{'='*60}")

    # 1. 跑算法
    print("[1/2] 运行 SOC 算法...")
    r = subprocess.run(
        [sys.executable, str(ROOT/"run_soc_sim.py"), str(csv_path)],
        capture_output=True, text=True, timeout=300, cwd=str(ROOT)
    )
    if r.returncode != 0:
        print(f"ERROR: {r.stderr[-300:]}")
        return False
    print("  OK")

    # 2. 生成图
    print("[2/2] 生成交互式图表...")
    r = subprocess.run(
        [sys.executable, str(ROOT/"plot_soc2.py"), str(result_csv)],
        capture_output=True, text=True, timeout=60, cwd=str(ROOT)
    )
    if r.returncode != 0:
        print(f"ERROR: {r.stderr[-300:]}")
        return False
    print("  OK")
    print(f"\n  CSV  : {result_csv}")
    print(f"  HTML : {result_html}")
    return True


def main():
    if len(sys.argv) < 2:
        print(f"用法: python {sys.argv[0]} <csv_name|all>")
        sys.exit(1)

    arg = sys.argv[1]

    if arg.lower() == "all":
        files = sorted(MS100.glob("*.csv"))
        # 跳过之前的结果文件
        files = [f for f in files if "_soc_result" not in f.name and "_summary" not in f.name and "_error" not in f.name]
        ok = 0
        for f in files:
            if run_one(f):
                ok += 1
        print(f"\n{'='*60}")
        print(f"完成: {ok}/{len(files)}")
    else:
        # 单个文件
        csv_path = MS100 / f"{arg}.csv"
        if not csv_path.exists():
            print(f"找不到文件: {csv_path}")
            sys.exit(1)
        run_one(csv_path)


if __name__ == "__main__":
    main()
