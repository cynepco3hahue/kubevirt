pipeline {
  agent any
  stages {
    stage('Tests') {
      parallel {
        stage('k8s-1.10.3-dev') {
          environment {
            TARGET = 'k8s-1.10.3-dev'
          }
          steps {
            sh '''#!/bin/bash
set -o pipefail
bash automation/test.sh 2>&1 | tee ${WORKSPACE}/k8s-1.10.3-dev-console.log'''
          }
        }
        stage('k8s-1.10.3-release') {
          environment {
            TARGET = 'k8s-1.10.3-release'
          }
          steps {
            sh '''#!/bin/bash
set -o pipefail
bash automation/test.sh 2>&1 | tee ${WORKSPACE}/k8s-1.10.3-dev-console.log'''
          }
        }
      }
    }
  }
}