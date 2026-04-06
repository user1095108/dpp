import time
import dpp
import decimal
from decimal import Decimal

def run_benchmark(type_name, iterations=3_000_000):
    prec_map = {"d32": 7, "d64": 16, "d128": 34}
    target_prec = prec_map.get(type_name, 28)
    
    decimal.getcontext().prec = target_prec
    print(f"--- Benchmarking {type_name} (Precision: {decimal.getcontext().prec} digits) ---")

    val_str_a = "1.234567"
    val_str_b = "9.876543"

    py_a, py_b = Decimal(val_str_a), Decimal(val_str_b)
    start = time.perf_counter()
    for _ in range(iterations):
        res = (py_a + py_b) * py_a / py_b
    py_time = time.perf_counter() - start
    print(f"Python Decimal: {py_time:.4f}s")

    dpp_class = getattr(dpp, type_name)
    dpp_a, dpp_b = dpp_class(val_str_a), dpp_class(val_str_b)
    start = time.perf_counter()
    for _ in range(iterations):
        res = (dpp_a + dpp_b) * dpp_a / dpp_b
    dpp_time = time.perf_counter() - start
    print(f"dpp.{type_name}:        {dpp_time:.4f}s")

    print(f"Speedup:        {py_time / dpp_time:.2f}x\n")

if __name__ == "__main__":
    run_benchmark("d32")
    run_benchmark("d64")
    run_benchmark("d128")

