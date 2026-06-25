import os
import subprocess


def test_repo_regression(regression_case):
    repo_path = regression_case["path"]
    argv = regression_case["argv"]

    assert repo_path.is_dir(), f"missing repo path: {repo_path}"

    env = os.environ.copy()
    env.setdefault("PYTHONUNBUFFERED", "1")

    result = subprocess.run(
        argv,
        cwd=repo_path,
        env=env,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=900,
        check=False,
    )

    assert result.returncode == 0, (
        f"{regression_case['repo']}::{regression_case['name']} failed\n"
        f"cwd: {repo_path}\n"
        f"cmd: {' '.join(argv)}\n"
        f"output:\n{result.stdout}"
    )
