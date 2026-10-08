#pragma once

int TrainButton(struct OCR* ocr);
int CreateNetworkButton(struct OCR* ocr);
int LoadNetworkButton(struct OCR* ocr);
void LearningRateInterface(struct OCR* ocr, struct nk_context* ctx);
void NeuronsPerLayerInterface(struct OCR* ocr, struct nk_context* ctx);
void TrainingCycleInterface(struct OCR* ocr, struct nk_context* ctx);
void RotateInterface(struct OCR* ocr, struct nk_context* ctx);
