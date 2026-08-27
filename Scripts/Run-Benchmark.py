#!/usr/bin/env python3
"""
UTween Performance Benchmark Runner
Executes UTween automation benchmarks via Unreal MCP or headlessly via UnrealEditor-Cmd.exe.
Parses logs and reports comprehensive performance metrics and bottleneck diagnostics.
"""

import sys
import os
import json
import time
import re
import urllib.request
import urllib.error
import subprocess

MCP_URL = "http://127.0.0.1:8000/mcp"
BENCHMARK_TESTS = [
    "UTween.Benchmark.Easing",
    "UTween.Benchmark.TweenCreation",
    "UTween.Benchmark.TickThroughput",
    "UTween.Benchmark.ActorAndWidget",
    "UTween.Benchmark.SequenceScale",
    "UTween.Benchmark.ChurnAndCleanup",
    "UTween.Benchmark.TagFiltering"
]

def rpc(method, params, session_id=None):
    payload = {"jsonrpc": "2.0", "id": 1, "method": method, "params": params}
    headers = {'Content-Type': 'application/json'}
    if session_id:
        headers['Mcp-Session-Id'] = session_id
    req = urllib.request.Request(MCP_URL, data=json.dumps(payload).encode('utf-8'), headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            sid = resp.headers.get("Mcp-Session-Id") or session_id
            return sid, json.loads(resp.read().decode('utf-8'))
    except Exception as e:
        return None, None

def check_mcp_available():
    sid, res = rpc("initialize", {
        "protocolVersion": "2024-11-05",
        "capabilities": {},
        "clientInfo": {"name": "benchmark-runner", "version": "1.0"}
    })
    return sid is not None, sid

def run_via_mcp(sid):
    print(">> Initializing test discovery via Unreal MCP...")
    sid, _ = rpc("tools/call", {
        "name": "call_tool",
        "arguments": {
            "toolset_name": "AutomationTestToolset.AutomationTestToolset",
            "tool_name": "DiscoverTests",
            "arguments": {}
        }
    }, sid)
    time.sleep(1.0)

    print(f">> Triggering {len(BENCHMARK_TESTS)} benchmark tests via Unreal MCP...")
    sid, run_res = rpc("tools/call", {
        "name": "call_tool",
        "arguments": {
            "toolset_name": "AutomationTestToolset.AutomationTestToolset",
            "tool_name": "RunTests",
            "arguments": {"testNames": BENCHMARK_TESTS}
        }
    }, sid)

    print(">> Waiting for benchmarks to complete...")
    start_wait = time.time()
    while time.time() - start_wait < 120:
        time.sleep(1.5)
        sid, status_res = rpc("tools/call", {
            "name": "call_tool",
            "arguments": {
                "toolset_name": "AutomationTestToolset.AutomationTestToolset",
                "tool_name": "GetTestStatus",
                "arguments": {}
            }
        }, sid)
        if status_res and "result" in status_res:
            raw_text = status_res["result"]["content"][0]["text"]
            try:
                ret_val = json.loads(raw_text).get("returnValue", "{}")
                status = json.loads(ret_val)
                state = status.get("state")
                num_complete = status.get("numComplete", 0)
                num_total = status.get("numEnabled", len(BENCHMARK_TESTS))
                print(f"   Progress: {num_complete}/{num_total} completed (State: {state})")
                if state in ["Ready", "NotStarted", "Done"] and num_complete >= len(BENCHMARK_TESTS):
                    break
            except Exception:
                pass

    print(">> Fetching test results...")
    sid, res_data = rpc("tools/call", {
        "name": "call_tool",
        "arguments": {
            "toolset_name": "AutomationTestToolset.AutomationTestToolset",
            "tool_name": "GetTestResults",
            "arguments": {}
        }
    }, sid)

    print(">> Fetching benchmark output logs...")
    sid, log_res = rpc("tools/call", {
        "name": "call_tool",
        "arguments": {
            "toolset_name": "EditorToolset.LogsToolset",
            "tool_name": "GetLogEntries",
            "arguments": {"category": "LogUTween", "pattern": ".*", "maxEntries": 500}
        }
    }, sid)

    raw_logs = []
    if log_res and "result" in log_res:
        log_text = log_res["result"]["content"][0]["text"]
        try:
            raw_logs = json.loads(log_text).get("returnValue", [])
        except Exception:
            pass

    return raw_logs, res_data

def run_headless():
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    uproject = os.path.join(project_root, "UTween.uproject")
    engine_root = r"C:\Program Files\Epic Games\UE_5.8"
    editor_cmd = os.path.join(engine_root, r"Engine\Binaries\Win64\UnrealEditor-Cmd.exe")

    if not os.path.exists(editor_cmd):
        raise RuntimeError(f"UnrealEditor-Cmd.exe not found at {editor_cmd}")

    print(f">> Executing benchmarks headlessly via {editor_cmd}...")
    cmd = [
        editor_cmd,
        uproject,
        '-ExecCmds=Automation RunTests UTween.Benchmark; Quit',
        '-stdout',
        '-FullStdOutLogOutput',
        '-unattended',
        '-NullRHI',
        '-NoPause',
        '-buildmachine'
    ]

    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)
    captured_logs = []
    for line in proc.stdout:
        print(line, end="")
        if "LogUTween" in line or "UTween Benchmark" in line:
            captured_logs.append(line)
    proc.wait()
    return captured_logs, None

def parse_and_report(logs):
    print("\n" + "=" * 80)
    print("                   UTWEEN BENCHMARK PERFORMANCE REPORT")
    print("=" * 80)

    easing_lines = []
    creation_lines = []
    tick_lines = []
    target_lines = []
    sequence_lines = []
    churn_lines = []
    tag_lines = []

    for line in logs:
        text = line.strip()
        if "Ease [" in text:
            easing_lines.append(text)
        elif "Creation [" in text:
            creation_lines.append(text)
        elif "Tick [" in text:
            tick_lines.append(text)
        elif "Baseline (" in text or "ActorLocation (" in text or "WidgetPosition (" in text:
            target_lines.append(text)
        elif "Sequences [" in text:
            sequence_lines.append(text)
        elif "Churn Stress [" in text:
            churn_lines.append(text)
        elif "Tag Filtering:" in text:
            tag_lines.append(text)

    if easing_lines:
        print("\n1. EASING EVALUATION MICRO-BENCHMARK (1,000,000 evaluations each)")
        print("-" * 80)
        for l in easing_lines:
            match = re.search(r"Ease \[([^\]]+)\]:\s*Total\s*([\d\.]+)\s*ms\s*\|\s*([\d\.]+)\s*ns/op\s*\|\s*([\d\.]+)\s*Mops/s", l)
            if match:
                name, total_ms, ns_op, mops = match.groups()
                print(f"  {name:<18} | {total_ms:>7} ms total | {ns_op:>7} ns/op | {mops:>7} Mops/sec")
            else:
                print("  " + l)

    if creation_lines:
        print("\n2. TWEEN CREATION & REGISTRATION THROUGHPUT")
        print("-" * 80)
        for l in creation_lines:
            match = re.search(r"Creation \[([^\]]+)\]:\s*([\d\.]+)\s*ms\s*\|\s*([\d\.]+)\s*us/tween\s*\|\s*([\d\.]+)\s*tweens/s", l)
            if match:
                count, total_ms, us_tween, tps = match.groups()
                print(f"  {count:<18} | {total_ms:>7} ms total | {us_tween:>7} us/creation | {float(tps):>10,.0f} creations/sec")
            else:
                print("  " + l)

    if tick_lines:
        print("\n3. STEADY-STATE TICK THROUGHPUT STRESS TEST (100 frames @ 60 FPS)")
        print("-" * 80)
        for l in tick_lines:
            match = re.search(r"Tick \[([^\]]+)\]:\s*Avg\s*([\d\.]+)\s*ms/frame\s*\(Min:\s*([\d\.]+),\s*Max:\s*([\d\.]+)\)\s*\|\s*([\d\.]+)\s*us/tween/frame\s*\|\s*([\d\.]+)\s*evals/s", l)
            if match:
                count, avg_ms, min_ms, max_ms, us_tf, evals = match.groups()
                print(f"  {count:<18} | Avg: {avg_ms:>6} ms (Min:{min_ms:>5}, Max:{max_ms:>5}) | {us_tf:>6} us/tween/frame | {float(evals):>11,.0f} evals/sec")
            else:
                print("  " + l)

    if target_lines:
        print("\n4. TARGET BINDING & WEAK POINTER RESOLUTION OVERHEAD (2,000 instances)")
        print("-" * 80)
        for l in target_lines:
            print("  " + l)

    if sequence_lines:
        print("\n5. SEQUENCE TIMELINE & ACTION SLICING BENCHMARK")
        print("-" * 80)
        for l in sequence_lines:
            print("  " + l)

    if churn_lines:
        print("\n6. CHURN & CLEANUP STRESS TEST (500 spawns/frame + completions)")
        print("-" * 80)
        for l in churn_lines:
            print("  " + l)

    if tag_lines:
        print("\n7. SUBSYSTEM TAG FILTERING BENCHMARK (10,000 active tweens)")
        print("-" * 80)
        for l in tag_lines:
            print("  " + l)

    print("\n" + "=" * 80)

def main():
    force_headless = "--headless" in sys.argv
    mcp_avail = False
    sid = None

    if not force_headless:
        mcp_avail, sid = check_mcp_available()

    if mcp_avail:
        print(">> Detected active Unreal Engine MCP session on port 8000.")
        logs, res = run_via_mcp(sid)
        parse_and_report(logs)
    else:
        print(">> Running benchmarks headlessly via UnrealEditor-Cmd...")
        logs, _ = run_headless()
        parse_and_report(logs)

if __name__ == "__main__":
    main()
