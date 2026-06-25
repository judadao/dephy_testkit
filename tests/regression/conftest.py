import json
from pathlib import Path

import pytest


WORKSPACE_ROOT = Path(__file__).resolve().parents[3]
MANIFEST = Path(__file__).with_name("regression_manifest.json")


def pytest_addoption(parser):
    parser.addoption(
        "--module",
        action="append",
        default=[],
        help="Run only the named repo/module. Can be repeated.",
    )
    parser.addoption(
        "--profile",
        default="quick",
        choices=("quick", "integration", "full"),
        help="Regression profile to run.",
    )


def _load_cases(config):
    selected_modules = set(config.getoption("--module"))
    selected_profile = config.getoption("--profile")
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    cases = []

    for repo in data["repos"]:
        repo_name = repo["name"]
        if selected_modules and repo_name not in selected_modules:
            continue

        repo_path = WORKSPACE_ROOT / repo["path"]
        for command in repo["commands"]:
            profiles = set(command["profiles"])
            if selected_profile not in profiles:
                continue

            case = {
                "repo": repo_name,
                "path": repo_path,
                "name": command["name"],
                "argv": command["argv"],
                "profile": selected_profile,
            }
            cases.append(case)

    return cases


def pytest_generate_tests(metafunc):
    if "regression_case" not in metafunc.fixturenames:
        return

    cases = _load_cases(metafunc.config)
    ids = [f"{case['repo']}::{case['name']}" for case in cases]
    metafunc.parametrize("regression_case", cases, ids=ids)
