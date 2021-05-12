@Library('xmos_jenkins_shared_library@v0.14.2') _
getApproval()
pipeline {
    agent {
        dockerfile {
            args ""
        }
    }
    parameters { // Available to modify on the job page within Jenkins if starting a build
        string( // use to try different tools versions
            name: 'TOOLS_VERSION',
            defaultValue: '15.0.6',
            description: 'The tools version to build with (check /projects/tools/ReleasesTools/)'
        )
        booleanParam( // use to check results of rolling all conda deps forward
            name: 'UPDATE_ALL',
            defaultValue: false,
            description: 'Update all conda packages before building'
        )
    }
    options { // plenty of things could go here
        //buildDiscarder(logRotator(numToKeepStr: '10'))
        timestamps()
    }
    environment {
        XMOS_AIOT_SDK_PATH = "${env.WORKSPACE}"
    }
    stages {
        stage("Setup") {
            // Clone and install build dependencies
            steps {
                // clean auto default checkout
                sh "rm -rf *"
                // clone
                checkout([
                    $class: 'GitSCM',
                    branches: scm.branches,
                    doGenerateSubmoduleConfigurations: false,
                    extensions: [[$class: 'SubmoduleOption',
                                threads: 8,
                                timeout: 20,
                                shallow: false,
                                parentCredentials: true,
                                recursiveSubmodules: true],
                                [$class: 'CleanCheckout']],
                    userRemoteConfigs: [[credentialsId: 'xmos-bot',
                                        url: 'git@github.com:xmos/aiot_sdk']]
                ])
                // create venv
                sh "conda env create -q -p sdk_venv -f environment.yml"
                // Install xmos tools version
                sh "/XMOS/get_tools.py " + params.TOOLS_VERSION
            }
        }
        stage("Update environment") {
            // Roll all conda packages forward beyond their pinned versions
            when { expression { return params.UPDATE_ALL } }
            steps {
                sh "conda update --all -y -q -p sdk_venv"
            }
        }
        stage("Build examples") {
            steps {
                sh """. /XMOS/tools/${params.TOOLS_VERSION}/XMOS/XTC/${params.TOOLS_VERSION}/SetEnv &&
                      . activate ./sdk_venv && bash test/build_examples.sh"""
            }
        }
        stage("Install") {
            steps {
                sh """. activate ./sdk_venv && bash install.sh"""
            }
        }
        stage("Test") {
            steps {
                // run unit tests
                sh """. activate ./sdk_venv && cd test && pytest -v --junitxml tests_junit.xml"""
                // run notebook tests
                sh """. activate ./sdk_venv && cd test && bash test_notebooks.sh"""
                // Any call to pytest can be given the "--junitxml SOMETHING_junit.xml" option
                // This step collects these files for display in Jenkins UI
                junit "**/*_junit.xml"
            }
        }
        stage("Build documentation") {
            steps {
                dir('documents') {
                    sh '. activate ../sdk_venv && make clean linkcheck html SPHINXOPTS="-W --keep-going"'
                    dir('_build') {
                        archiveArtifacts artifacts: 'html/**/*', fingerprint: false
                        sh 'tar -czf docs_sdk.tgz html'
                        archiveArtifacts artifacts: 'docs_sdk.tgz', fingerprint: true
                    }
                }
            }
        }
    }
    post {
        cleanup {
            cleanWs()
        }
    }
}
