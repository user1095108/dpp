import decimal
from decimal import Decimal
import dpp
import gc
import sys
import time

def run_benchmark(type_name, iterations=3_000_000, repeats=10):
    prec_map = {"d32": 7, "d64": 16, "d128": 34}
    target_prec = prec_map.get(type_name, 28)
    decimal.getcontext().prec = target_prec

    print(f"--- Benchmarking {type_name} (Best of {repeats} runs) ---")

    val_str_a, val_str_b = "1.234567", "9.876543"

    def get_best_time(a, b):
        t = sys.float_info.max

        for _ in range(iterations // 10):
            res = (a + b) * a / b

        for _ in range(repeats):
            start = time.perf_counter()
            for _ in range(iterations):
                res = (a + b) * a / b
            t = min(t, time.perf_counter() - start)

        return t

    gc.collect()
    gc.disable()

    py_a, py_b = Decimal(val_str_a), Decimal(val_str_b)
    py_time = get_best_time(py_a, py_b)
    print(f"{'Python Decimal':<{16}}: {py_time:.4f}s")

    dpp_class = getattr(dpp, type_name)
    dpp_a, dpp_b = dpp_class(val_str_a), dpp_class(val_str_b)
    dpp_time = get_best_time(dpp_a, dpp_b)
    print(f"{f'dpp.{type_name}':<{16}}: {dpp_time:.4f}s")

    gc.enable()

    print(f"{'Speedup':<{16}}: {py_time / dpp_time:.2f}x\n")

if __name__ == "__main__":
    for t in ["d32", "d64", "d128"]:
        run_benchmark(t)
