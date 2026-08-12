#!/usr/bin/env python3

import argparse
import json
import os
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path


def read_json_file(path: Path):
    if not path.exists() or path.stat().st_size == 0:
        return {}
    try:
        with path.open("r", encoding="utf-8") as fh:
            return json.load(fh)
    except json.JSONDecodeError:
        return {}


def write_json_file(path: Path, payload):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as fh:
        json.dump(payload, fh, indent=2)
        fh.write("\n")


def parse_benchmark_stdout(stdout: str):
    payloads = []

    for raw_line in stdout.splitlines():
        line = raw_line.strip()
        if "BENCHMARK_RESULT:" not in line:
            continue

        payload = line.split("BENCHMARK_RESULT:", 1)[1].strip()
        payloads.append(json.loads(payload))

    return payloads


def git_history_baseline(repo_root: Path, history_path: Path):
    if not repo_root.exists():
        return None

    try:
        subprocess.run(["git", "rev-parse", "--is-inside-work-tree"], cwd=repo_root, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except (FileNotFoundError, subprocess.CalledProcessError):
        return None

    if history_path.exists():
        history = read_json_file(history_path)
        baseline_ms = history.get("baseline_ms")
        if baseline_ms is not None:
            return float(baseline_ms)

    try:
        result = subprocess.run(
            ["git", "show", "origin/main:" + str(history_path.relative_to(repo_root))],
            cwd=repo_root,
            check=True,
            capture_output=True,
            text=True,
        )
    except (FileNotFoundError, subprocess.CalledProcessError):
        return None

    try:
        baseline_history = json.loads(result.stdout)
        baseline_ms = baseline_history.get("baseline_ms")
        if baseline_ms is not None:
            return float(baseline_ms)
    except json.JSONDecodeError:
        return None

    return None


def build_svg_chart(output_path: Path, current_ms: float, baseline_ms: float | None, threshold_percent: float):
    output_path.parent.mkdir(parents=True, exist_ok=True)

    chart_height = 420
    chart_width = 820
    margin_left = 80
    margin_right = 60
    margin_top = 50
    margin_bottom = 70
    plot_width = chart_width - margin_left - margin_right
    plot_height = chart_height - margin_top - margin_bottom

    max_value = max(current_ms, baseline_ms or current_ms, 1.0)
    max_value = max_value * 1.2
    baseline_value = baseline_ms if baseline_ms is not None else current_ms
    threshold_value = baseline_value * (1.0 + (threshold_percent / 100.0))

    def y_for(value: float) -> float:
        return chart_height - margin_bottom - ((value / max_value) * plot_height)

    labels = [
        "baseline",
        "current",
    ]
    values = [baseline_value, current_ms]
    color_map = ["#7aa2f7", "#bb9af7"]

    svg_lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{chart_width}" height="{chart_height}" viewBox="0 0 {chart_width} {chart_height}">',
        '<rect width="100%" height="100%" fill="#111827"/>',
        f'<text x="{chart_width / 2}" y="26" text-anchor="middle" font-size="22" fill="#e5e7eb" font-family="Arial">Log print performance benchmark</text>',
        f'<line x1="{margin_left}" y1="{chart_height - margin_bottom}" x2="{chart_width - margin_right}" y2="{chart_height - margin_bottom}" stroke="#9ca3af" stroke-width="1.5"/>',
        f'<line x1="{margin_left}" y1="{margin_top}" x2="{margin_left}" y2="{chart_height - margin_bottom}" stroke="#9ca3af" stroke-width="1.5"/>',
        f'<line x1="{margin_left}" y1="{y_for(threshold_value)}" x2="{chart_width - margin_right}" y2="{y_for(threshold_value)}" stroke="#fbbf24" stroke-dasharray="7 5" stroke-width="1.5"/>',
        f'<text x="{chart_width - margin_right}" y="{y_for(threshold_value) - 8}" text-anchor="end" font-size="12" fill="#fbbf24" font-family="Arial">10% threshold</text>',
    ]

    for idx, label in enumerate(labels):
        x = margin_left + idx * (plot_width / 2) + 50
        y = y_for(values[idx])
        height = chart_height - margin_bottom - y
        svg_lines.append(f'<rect x="{x}" y="{y}" width="120" height="{height}" fill="{color_map[idx]}" rx="6"/>')
        svg_lines.append(f'<text x="{x + 60}" y="{chart_height - margin_bottom + 24}" text-anchor="middle" font-size="13" fill="#e5e7eb" font-family="Arial">{label}</text>')
        svg_lines.append(f'<text x="{x + 60}" y="{y - 10}" text-anchor="middle" font-size="12" fill="#f8fafc" font-family="Arial">{values[idx]:.2f} ms</text>')

    for tick in range(0, 6):
        value = (max_value / 5.0) * tick
        y = chart_height - margin_bottom - ((value / max_value) * plot_height)
        svg_lines.append(f'<line x1="{margin_left}" y1="{y}" x2="{chart_width - margin_right}" y2="{y}" stroke="#374151" stroke-width="1"/>')
        svg_lines.append(f'<text x="{margin_left - 12}" y="{y + 4}" text-anchor="end" font-size="11" fill="#cbd5e1" font-family="Arial">{value:.0f}</text>')

    svg_lines.append('</svg>')
    output_path.write_text("\n".join(svg_lines), encoding="utf-8")


def build_sink_chart(output_path: Path, records):
    output_path.parent.mkdir(parents=True, exist_ok=True)

    sinks = ["console", "file", "console_and_file"]
    lengths = sorted({int(item["message_length"]) for item in records})
    chart_width = 920
    chart_height = 520
    margin_left = 90
    margin_right = 40
    margin_top = 60
    margin_bottom = 100
    plot_width = chart_width - margin_left - margin_right
    plot_height = chart_height - margin_top - margin_bottom
    max_value = max(float(item["logs_per_second"]) for item in records) * 1.2
    colors = {"console": "#7aa2f7", "file": "#34d399", "console_and_file": "#fbbf24"}

    svg_lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{chart_width}" height="{chart_height}" viewBox="0 0 {chart_width} {chart_height}">',
        '<rect width="100%" height="100%" fill="#111827"/>',
        f'<text x="{chart_width / 2}" y="28" text-anchor="middle" font-size="22" fill="#e5e7eb" font-family="Arial">Throughput by output sink and message length</text>',
        f'<line x1="{margin_left}" y1="{chart_height - margin_bottom}" x2="{chart_width - margin_right}" y2="{chart_height - margin_bottom}" stroke="#9ca3af" stroke-width="1.5"/>',
        f'<line x1="{margin_left}" y1="{margin_top}" x2="{margin_left}" y2="{chart_height - margin_bottom}" stroke="#9ca3af" stroke-width="1.5"/>',
    ]

    group_width = plot_width / len(lengths)
    bar_width = group_width / (len(sinks) + 1)

    for tick in range(0, 6):
        value = (max_value / 5.0) * tick
        y = chart_height - margin_bottom - ((value / max_value) * plot_height)
        svg_lines.append(f'<line x1="{margin_left}" y1="{y}" x2="{chart_width - margin_right}" y2="{y}" stroke="#374151" stroke-width="1"/>')
        svg_lines.append(f'<text x="{margin_left - 12}" y="{y + 4}" text-anchor="end" font-size="11" fill="#cbd5e1" font-family="Arial">{value:.0f}</text>')

    for idx, message_length in enumerate(lengths):
        x0 = margin_left + idx * group_width + 18
        svg_lines.append(f'<text x="{x0 + group_width / 2 - 10}" y="{chart_height - margin_bottom + 22}" text-anchor="middle" font-size="12" fill="#e5e7eb" font-family="Arial">{message_length}B</text>')

        for sink_idx, sink_name in enumerate(sinks):
            value = next((float(item["logs_per_second"]) for item in records if item["sink"] == sink_name and int(item["message_length"]) == message_length), 0.0)
            x = x0 + sink_idx * bar_width + 8
            bar_height = (value / max_value) * plot_height
            y = chart_height - margin_bottom - bar_height
            color = colors.get(sink_name, "#ffffff")
            svg_lines.append(f'<rect x="{x}" y="{y}" width="{bar_width - 8}" height="{bar_height}" fill="{color}" rx="4"/>')
            svg_lines.append(f'<text x="{x + (bar_width - 10) / 2}" y="{y - 8}" text-anchor="middle" font-size="10" fill="#f8fafc" font-family="Arial">{value:.0f}</text>')

    legend_y = margin_top - 8
    for sink_name in sinks:
        label_x = margin_left + 30 + sinks.index(sink_name) * 140
        svg_lines.append(f'<rect x="{label_x}" y="{legend_y - 12}" width="14" height="14" fill="{colors[sink_name]}" rx="3"/>')
        svg_lines.append(f'<text x="{label_x + 20}" y="{legend_y}" font-size="12" fill="#e5e7eb" font-family="Arial">{sink_name}</text>')

    svg_lines.append('</svg>')
    output_path.write_text("\n".join(svg_lines), encoding="utf-8")


def run_benchmark(binary_path: Path, test_filter: str = "PerformanceBenchmark.LogPrintThroughput"):
    resolved_binary = binary_path.expanduser().resolve()
    command = [str(resolved_binary), f"--gtest_filter={test_filter}", "--gtest_brief=1"]
    result = subprocess.run(command, capture_output=True, text=True, cwd=str(resolved_binary.parent), check=False)
    stdout = (result.stdout or "") + (result.stderr or "")
    benchmark_results = parse_benchmark_stdout(stdout)

    if not benchmark_results and "SKIPPED" in stdout:
        return [], result.returncode

    return benchmark_results, result.returncode


def main():
    parser = argparse.ArgumentParser(description="Run the EquinoxLogger logging performance benchmarks and compare them to the baseline history.")
    parser.add_argument("--binary", type=Path, required=True, help="Path to the EquinoxLoggerTests.x86 binary.")
    parser.add_argument("--output-dir", type=Path, default=Path("docs/images"), help="Directory for generated chart output.")
    parser.add_argument("--chart-name", type=str, default="performance-benchmark.svg", help="Output filename for the generated baseline SVG chart.")
    parser.add_argument("--sink-chart-name", type=str, default="performance-throughput-by-sink.svg", help="Output filename for the sink/message-length throughput SVG chart.")
    parser.add_argument("--history-file", type=Path, default=Path("benchmarks/performance-history.json"), help="Path to the JSON baseline history file.")
    parser.add_argument("--threshold", type=float, default=10.0, help="Maximum allowed slowdown percentage before the benchmark fails.")
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parents[1], help="Repository root used for Git baseline lookup.")
    args = parser.parse_args()

    repo_root = args.repo_root.expanduser().resolve()
    binary_path = args.binary.expanduser().resolve() if args.binary.is_absolute() else (repo_root / args.binary).resolve()
    output_dir = args.output_dir if args.output_dir.is_absolute() else (repo_root / args.output_dir)
    history_file = args.history_file if args.history_file.is_absolute() else (repo_root / args.history_file)

    if not binary_path.exists():
        print(f"Benchmark binary not found: {binary_path}", file=sys.stderr)
        return 2

    benchmark_result = None
    sink_records = []
    overall_exit_code = 0

    for test_filter in [
        "PerformanceBenchmark.LogPrintThroughput",
        "PerformanceBenchmark.ThroughputBySinkAndMessageLength_Console",
        "PerformanceBenchmark.ThroughputBySinkAndMessageLength_File",
        "PerformanceBenchmark.ThroughputBySinkAndMessageLength_ConsoleAndFile",
    ]:
        partial_results, exit_code = run_benchmark(binary_path, test_filter)
        overall_exit_code = exit_code if exit_code != 0 else overall_exit_code

        if test_filter == "PerformanceBenchmark.LogPrintThroughput":
            if partial_results:
                benchmark_result = partial_results[0]
        else:
            sink_records.extend(item for item in partial_results if "logs_per_second" in item)

    output_dir.mkdir(parents=True, exist_ok=True)

    if sink_records:
        sink_chart_path = output_dir / args.sink_chart_name
        build_sink_chart(sink_chart_path, sink_records)
        print(f"Generated throughput chart: {sink_chart_path}")
        for item in sink_records:
            print(f"Sink={item['sink']} len={item['message_length']} logs/sec={float(item['logs_per_second']):.2f}")

    if benchmark_result is None:
        return overall_exit_code

    current_ms = float(benchmark_result["elapsed_ms"])

    baseline_ms = git_history_baseline(repo_root, history_file)
    if baseline_ms is None:
        history_payload = read_json_file(history_file)
        baseline_ms = float(history_payload.get("baseline_ms", current_ms))

    delta_percent = 0.0 if baseline_ms <= 0 else ((current_ms - baseline_ms) / baseline_ms) * 100.0
    threshold_value = baseline_ms * (1.0 + (args.threshold / 100.0))

    chart_path = output_dir / args.chart_name
    build_svg_chart(chart_path, current_ms, baseline_ms, args.threshold)

    history_payload = {
        "baseline_ms": baseline_ms,
        "current_ms": current_ms,
        "delta_percent": round(delta_percent, 2),
        "threshold_percent": args.threshold,
        "threshold_value_ms": round(threshold_value, 2),
        "history": [
            {"label": "baseline", "value_ms": round(baseline_ms, 2)},
            {"label": "current", "value_ms": round(current_ms, 2)},
        ],
        "last_updated_utc": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
    }
    write_json_file(history_file, history_payload)

    print(f"Benchmark current: {current_ms:.2f} ms")
    print(f"Benchmark baseline: {baseline_ms:.2f} ms")
    print(f"Benchmark change: {delta_percent:.2f}%")
    print(f"Threshold: {args.threshold}%")
    print(f"Chart written to: {chart_path}")

    if delta_percent > args.threshold:
        print(f"Performance regression exceeds {args.threshold}% threshold ({delta_percent:.2f}% slower).", file=sys.stderr)
        return 1

    if overall_exit_code != 0:
        return overall_exit_code

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
