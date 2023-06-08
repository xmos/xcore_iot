@Library('xmos_jenkins_shared_library@v0.20.0') _

getApproval()

pipeline {
    agent {
        label 'xcore.ai-explorer-us'
    }
    options {
        disableConcurrentBuilds()
        skipDefaultCheckout()
        timestamps()
        // on develop discard builds after a certain number else keep forever
        buildDiscarder(logRotator(
            numToKeepStr:         env.BRANCH_NAME ==~ /develop/ ? '25' : '',
            artifactNumToKeepStr: env.BRANCH_NAME ==~ /develop/ ? '25' : ''
        ))
    }    
    parameters {
        string(
            name: 'TOOLS_VERSION',
            defaultValue: '15.2.1',
            description: 'The XTC tools version'
        )
    }    
    environment {
        PYTHON_VERSION = "3.8.11"
        VENV_DIRNAME = ".venv"
        BUILD_DIRNAME = "dist"
        TEST_RIG_TARGET = "xcore_sdk_test_rig"
    }        
    stages {
        stage('Checkout') {
            when {
                expression { params.NIGHTLY_TEST_ONLY == true }
            }
            steps {
                checkout scm
                sh 'git submodule update --init --recursive --depth 1 --jobs \$(nproc)'
            }
        }
        stage('Build applications and firmware') {
            when {
                expression { params.NIGHTLY_TEST_ONLY == true }
            }
            steps {
                script {
                    uid = sh(returnStdout: true, script: 'id -u').trim()
                    gid = sh(returnStdout: true, script: 'id -g').trim()
                }
                // pull docker image
                sh "docker pull ghcr.io/xmos/xcore_builder:latest"
                // host apps
                sh "docker run --rm -u $uid:$gid -w /xcore_iot -v $WORKSPACE:/xcore_iot ghcr.io/xmos/xcore_builder:latest bash -l tools/ci/build_host_apps.sh"
                // test firmware and filesystems
                sh "docker run --rm -u $uid:$gid -w /xcore_iot -v $WORKSPACE:/xcore_iot ghcr.io/xmos/xcore_builder:latest bash -l tools/ci/build_rtos_core_examples.sh"
                // List built files for log
                sh "ls -la dist_host/"
                sh "ls -la dist/"
            }
        }        
        stage('Create virtual environment') {
            when {
                expression { params.NIGHTLY_TEST_ONLY == true }
            }
            steps {
                // Create venv
                sh "pyenv install -s $PYTHON_VERSION"
                sh "~/.pyenv/versions/$PYTHON_VERSION/bin/python -m venv $VENV_DIRNAME"
                // Install dependencies
                withVenv() {
                    // NOTE: only one dependency so not using a requirements.txt file here yet
                    sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
                }
            }
        }
        stage('Cleanup xtagctl') {
            when {
                expression { params.NIGHTLY_TEST_ONLY == true }
            }
            steps {
                // Cleanup any xtagctl cruft from previous failed runs
                withTools(params.TOOLS_VERSION) {
                    withVenv {
                        sh "xtagctl reset_all $TEST_RIG_TARGET"
                    }
                }
                sh "rm -f ~/.xtag/status.lock ~/.xtag/acquired"
            }
        }
        stage('Run FreeRTOS examples') {
            when {
                expression { params.NIGHTLY_TEST_ONLY == true }
            }
            steps {
                withTools(params.TOOLS_VERSION) {
                    withVenv {
                        script {
                            if (fileExists("$BUILD_DIRNAME/example_freertos_getting_started.xe")) {
                                withXTAG(["$TEST_RIG_TARGET"]) { adapterIDs ->
                                    sh "test/examples/run_freertos_getting_started_tests.sh " + adapterIDs[0]
                                }
                            } else {
                                echo 'SKIPPED: example_freertos_getting_started'
                            }
                        } 
                        script {
                            if (fileExists("$BUILD_DIRNAME/example_freertos_explorer_board.xe")) {
                                withXTAG(["$TEST_RIG_TARGET"]) { adapterIDs ->
                                    sh "test/examples/run_freertos_explorer_board_tests.sh " + adapterIDs[0]
                                }
                            } else {
                                echo 'SKIPPED: example_freertos_explorer_board'
                            }
                        } 
                        script {
                            if (fileExists("$BUILD_DIRNAME/example_freertos_l2_cache.xe")) {
                                withXTAG(["$TEST_RIG_TARGET"]) { adapterIDs ->
                                    sh "test/examples/run_freertos_l2_cache_tests.sh " + adapterIDs[0]
                                }
                            } else {
                                echo 'SKIPPED: example_freertos_l2_cache'
                            }
                        } 
                        script {
                            if (fileExists("$BUILD_DIRNAME/example_freertos_tracealyzer.xe")) {
                                withXTAG(["$TEST_RIG_TARGET"]) { adapterIDs ->
                                    sh "test/examples/run_freertos_tracealyzer_tests.sh " + adapterIDs[0]
                                }
                            } else {
                                echo 'SKIPPED: example_freertos_tracealyzer'
                            }
                        }
                    }
                }
            }
        }
    }
    post {
        cleanup {
            // cleanWs removes all output and artifacts of the Jenkins pipeline
            //   Comment out this post section to leave the workspace which can be useful for running items on the Jenkins agent. 
            //   However, beware that this pipeline will not run if the workspace is not manually cleaned.
            cleanWs()
        }
    }
}
