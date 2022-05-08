// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EntityInfoDialog.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <MapEditor/Scripting/EditorEntityStore.hpp>
#include <MapEditor/Scripting/EditorScriptedEntity.hpp>
#include <MapEditor/Widgets/Float2SpinBox.hpp>
#include <MapEditor/Widgets/Float3SpinBox.hpp>
#include <MapEditor/Widgets/Float4SpinBox.hpp>
#include <MapEditor/Widgets/Integer2SpinBox.hpp>
#include <MapEditor/Widgets/Integer3SpinBox.hpp>
#include <MapEditor/Widgets/Integer4SpinBox.hpp>
#include <Nazara/Core/TypeTag.hpp>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <bitset>
#include <limits>

static_assert(sizeof(bw::EntityId) == sizeof(qlonglong));
static_assert(std::is_signed_v<bw::EntityId> == std::is_signed_v<qlonglong>);

Q_DECLARE_METATYPE(Nz::Vector2f);
Q_DECLARE_METATYPE(Nz::Vector2i64);
Q_DECLARE_METATYPE(Nz::Vector3f);
Q_DECLARE_METATYPE(Nz::Vector3i64);
Q_DECLARE_METATYPE(Nz::Vector4f);
Q_DECLARE_METATYPE(Nz::Vector4i64);

namespace bw
{
	class ComboBoxPropertyDelegate : public QStyledItemDelegate
	{
		public:
			ComboBoxPropertyDelegate(std::vector<std::pair<QString, QVariant>> options) :
			m_options(std::move(options))
			{
			}

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, QVariant value) const
			{
				model->setData(index, value, Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override
			{
				QComboBox* editor = new QComboBox(parent);
				editor->setFrame(false);

				for (const auto& boxOption : m_options)
					editor->addItem(boxOption.first, boxOption.second);

				return editor;
			}

			QString displayText(const QVariant& value, const QLocale& /*locale*/) const override
			{
				for (int i = 0; i < int(m_options.size()); ++i)
				{
					if (m_options[i].second == value)
						return m_options[i].first;
				}

				return "<Error>";
			}

			QVariant RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole);
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				QComboBox* comboBox = static_cast<QComboBox*>(editor);

				QVariant value = RetrieveModelData(index);
				for (int i = 0; i < int(m_options.size()); ++i)
				{
					if (m_options[i].second == value)
					{
						comboBox->setCurrentIndex(i);
						return;
					}
				}

				comboBox->setCurrentIndex(0);
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				QComboBox* comboBox = static_cast<QComboBox*>(editor);

				ApplyModelData(model, index, m_options[comboBox->currentIndex()].second);
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
			{
				editor->setGeometry(option.rect);
			}

		private:
			std::vector<std::pair<QString, QVariant>> m_options;
	};

	class FloatPropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, float value) const
			{
				model->setData(index, value, Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override
			{
				QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
				editor->setDecimals(6);
				editor->setFrame(false);
				editor->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());

				return editor;
			}

			float RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).toFloat();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
				spinBox->interpretText();

				ApplyModelData(model, index, float(spinBox->value()));
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
			{
				editor->setGeometry(option.rect);
			}
	};
	
	class Float2PropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, const Nz::Vector2f& value) const
			{
				QVariant qvalue;
				qvalue.setValue(value);

				model->setData(index, qvalue, Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override
			{
				return new Float2SpinBox(Float2SpinBox::LabelMode::NoLabel, QBoxLayout::LeftToRight, parent);
			}

			QString displayText(const QVariant& value, const QLocale& locale) const override
			{
				Nz::Vector2f point = value.value<Nz::Vector2f>();
				return QString("(%1; %2)").arg(locale.toString(point.x)).arg(locale.toString(point.y));
			}

			Nz::Vector2f RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).value<Nz::Vector2f>();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				Float2SpinBox* spinBox = static_cast<Float2SpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				Float2SpinBox* spinBox = static_cast<Float2SpinBox*>(editor);

				ApplyModelData(model, index, spinBox->value());
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
			{
				editor->setGeometry(option.rect);
			}
	};
	
	class Float3PropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, const Nz::Vector3f& value) const
			{
				QVariant qvalue;
				qvalue.setValue(value);

				model->setData(index, qvalue, Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override
			{
				return new Float3SpinBox(Float3SpinBox::LabelMode::NoLabel, QBoxLayout::LeftToRight, parent);
			}

			QString displayText(const QVariant& value, const QLocale& locale) const override
			{
				Nz::Vector3f vec = value.value<Nz::Vector3f>();
				return QString("(%1; %2; %3)").arg(locale.toString(vec.x)).arg(locale.toString(vec.y)).arg(locale.toString(vec.z));
			}

			Nz::Vector3f RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).value<Nz::Vector3f>();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				Float3SpinBox* spinBox = static_cast<Float3SpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				Float3SpinBox* spinBox = static_cast<Float3SpinBox*>(editor);

				ApplyModelData(model, index, spinBox->value());
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
			{
				editor->setGeometry(option.rect);
			}
	};

	class Float4PropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, const Nz::Vector4f& value) const
			{
				QVariant qvalue;
				qvalue.setValue(value);

				model->setData(index, qvalue, Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override
			{
				return new Float4SpinBox(Float4SpinBox::LabelMode::NoLabel, QBoxLayout::LeftToRight, parent);
			}

			QString displayText(const QVariant& value, const QLocale& locale) const override
			{
				Nz::Vector4f vec = value.value<Nz::Vector4f>();
				return QString("(%1; %2; %3; %4)").arg(locale.toString(vec.x)).arg(locale.toString(vec.y)).arg(locale.toString(vec.z)).arg(locale.toString(vec.w));
			}

			Nz::Vector4f RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).value<Nz::Vector4f>();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				Float4SpinBox* spinBox = static_cast<Float4SpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				Float4SpinBox* spinBox = static_cast<Float4SpinBox*>(editor);

				ApplyModelData(model, index, spinBox->value());
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
			{
				editor->setGeometry(option.rect);
			}
	};

	class IntegerPropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, Nz::Int64 value) const
			{
				model->setData(index, int(value), Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override
			{
				//TODO: Subclass for int64
				QSpinBox* editor = new QSpinBox(parent);
				editor->setFrame(false);
				editor->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());

				return editor;
			}

			Nz::Int64 RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).toInt();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
				spinBox->interpretText();

				ApplyModelData(model, index, spinBox->value());
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
			{
				editor->setGeometry(option.rect);
			}
	};
	
	class Integer2PropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, const Nz::Vector2i64& value) const
			{
				QVariant qvalue;
				qvalue.setValue(value);

				model->setData(index, qvalue, Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override
			{
				return new Integer2SpinBox(Integer2SpinBox::LabelMode::NoLabel, QBoxLayout::LeftToRight, parent);
			}

			QString displayText(const QVariant& value, const QLocale& locale) const override
			{
				Nz::Vector2i64 vec = value.value<Nz::Vector2i64>();
				return QString("(%1; %2)").arg(locale.toString(qlonglong(vec.x))).arg(locale.toString(qlonglong(vec.y)));
			}

			Nz::Vector2i64 RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).value<Nz::Vector2i64>();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				Integer2SpinBox* spinBox = static_cast<Integer2SpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				Integer2SpinBox* spinBox = static_cast<Integer2SpinBox*>(editor);

				ApplyModelData(model, index, spinBox->value());
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
			{
				editor->setGeometry(option.rect);
			}
	};
	
	class Integer3PropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, const Nz::Vector3i64& value) const
			{
				QVariant qvalue;
				qvalue.setValue(value);

				model->setData(index, qvalue, Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override
			{
				return new Integer3SpinBox(Integer3SpinBox::LabelMode::NoLabel, QBoxLayout::LeftToRight, parent);
			}

			QString displayText(const QVariant& value, const QLocale& locale) const override
			{
				Nz::Vector3i64 vec = value.value<Nz::Vector3i64>();
				return QString("(%1; %2; %3)").arg(locale.toString(qlonglong(vec.x))).arg(locale.toString(qlonglong(vec.y))).arg(locale.toString(qlonglong(vec.z)));
			}

			Nz::Vector3i64 RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).value<Nz::Vector3i64>();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				Integer3SpinBox* spinBox = static_cast<Integer3SpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				Integer3SpinBox* spinBox = static_cast<Integer3SpinBox*>(editor);

				ApplyModelData(model, index, spinBox->value());
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
			{
				editor->setGeometry(option.rect);
			}
	};
	
	class Integer4PropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, const Nz::Vector4i64& value) const
			{
				QVariant qvalue;
				qvalue.setValue(value);

				model->setData(index, qvalue, Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override
			{
				return new Integer4SpinBox(Integer4SpinBox::LabelMode::NoLabel, QBoxLayout::LeftToRight, parent);
			}

			QString displayText(const QVariant& value, const QLocale& locale) const override
			{
				Nz::Vector4i64 vec = value.value<Nz::Vector4i64>();
				return QString("(%1; %2; %3; %4)").arg(locale.toString(qlonglong(vec.x))).arg(locale.toString(qlonglong(vec.y))).arg(locale.toString(qlonglong(vec.z))).arg(locale.toString(qlonglong(vec.w)));
			}

			Nz::Vector4i64 RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).value<Nz::Vector4i64>();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				Integer4SpinBox* spinBox = static_cast<Integer4SpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				Integer4SpinBox* spinBox = static_cast<Integer4SpinBox*>(editor);

				ApplyModelData(model, index, spinBox->value());
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
			{
				editor->setGeometry(option.rect);
			}
	};

	struct EntityInfoDialog::Delegates
	{
		std::optional<ComboBoxPropertyDelegate> comboBoxDelegate;
		FloatPropertyDelegate floatDelegate;
		Float2PropertyDelegate float2Delegate;
		Float3PropertyDelegate float3Delegate;
		Float4PropertyDelegate float4Delegate;
		IntegerPropertyDelegate intDelegate;
		Integer2PropertyDelegate int2Delegate;
		Integer3PropertyDelegate int3Delegate;
		Integer4PropertyDelegate int4Delegate;
	};


	template<bool IsArray>
	struct PropertyOverrider;

	template<>
	struct PropertyOverrider<true>
	{
		template<PropertyType P, typename V, typename CB>
		static void OverrideProperty(EntityInfoDialog* /*owner*/, int arraySize, int rowIndex, PropertyValueMap& properties, const std::string& keyName, V&& newValue, CB&& callback)
		{
			bool first = false;

			using T = PropertyArrayValue<P>;

			auto it = properties.find(keyName);
			if (it == properties.end())
			{
				it = properties.emplace(keyName, T(arraySize)).first;
				first = true;
			}

			T& propertyArray = std::get<T>(it.value());
			propertyArray[rowIndex] = std::forward<V>(newValue);

			callback(first, QString{});
		}
	};

	template<>
	struct PropertyOverrider<false>
	{
		template<PropertyType P, typename V, typename CB>
		static void OverrideProperty(EntityInfoDialog* owner, PropertyValueMap& properties, const std::string& keyName, V&& newValue, CB&& callback)
		{
			bool first = false;

			using T = PropertySingleValue<P>;

			auto it = properties.find(keyName);
			if (it == properties.end())
			{
				it = properties.emplace(keyName, T{}).first;
				first = true;
			}

			it.value() = T(std::forward<V>(newValue));

			callback(first, owner->ToString(it.value(), P));
		}
	};


	template<PropertyType P, typename D, typename SpinBox, typename SpinBox::LabelMode L>
	struct MultiSpinboxPropertyWidget
	{
		virtual D* GetDelegate(EntityInfoDialog* owner) = 0;
		virtual PropertyValueMap& GetProperties(EntityInfoDialog* owner) = 0;

		using UT = PropertyUnderlyingType_t<P>;

		template<typename V, typename CB>
		QTableView* SetupArray(EntityInfoDialog* owner, int arraySize, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertyArrayValue<P>;

			D* delegate = GetDelegate(owner);
			PropertyValueMap& properties = GetProperties(owner);

			QTableView* tableView = new QTableView;
			QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
			tableView->setItemDelegate(delegate);
			tableView->setModel(model);

			model->setHorizontalHeaderLabels({ owner->tr("Value") });

			if (propertyValue)
			{
				auto& propertyArray = std::get<T>(propertyValue->get());
				for (int i = 0; i < arraySize; ++i)
					delegate->ApplyModelData(model, model->index(i, 0), propertyArray[i]);
			}


			owner->connect(model, &QStandardItemModel::itemChanged, [=, &properties, keyName = std::move(keyName), callback = std::forward<CB>(callback)](QStandardItem* item)
			{
				UT value = delegate->RetrieveModelData(item->index());

				PropertyOverrider<true>::template OverrideProperty<P>(owner, arraySize, item->index().row(), properties, keyName, value, callback);
			});

			return tableView;
		}

		template<typename V, typename CB>
		SpinBox* Setup(EntityInfoDialog* owner, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertySingleValue<P>;

			PropertyValueMap& properties = GetProperties(owner);

			SpinBox* spinbox = new SpinBox(L, QBoxLayout::TopToBottom);
			if (propertyValue && std::holds_alternative<T>(propertyValue->get()))
				spinbox->setValue(*std::get<T>(propertyValue->get()));

			owner->connect(spinbox, &SpinBox::valueChanged, [=, &properties, keyName = std::move(keyName), callback = std::forward<CB>(callback)]()
			{
				UT value = spinbox->value();

				PropertyOverrider<false>::template OverrideProperty<P>(owner, properties, keyName, value, callback);
			});

			return spinbox;
		}
	};

	template<PropertyType P>
	struct TextPropertyWidget
	{
		virtual PropertyValueMap& GetProperties(EntityInfoDialog* owner) = 0;

		template<typename V, typename CB>
		QTableWidget* SetupArray(EntityInfoDialog* owner, int arraySize, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertyArrayValue<P>;

			PropertyValueMap& properties = GetProperties(owner);

			QTableWidget* table = new QTableWidget(arraySize, 1);
			table->setHorizontalHeaderLabels({ owner->tr("Value") });
			table->setSelectionMode(QAbstractItemView::NoSelection);
			table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

			if (propertyValue)
			{
				auto& propertyArray = std::get<T>(propertyValue->get());

				QAbstractItemModel* model = table->model();
				for (int i = 0; i < arraySize; ++i)
					model->setData(model->index(i, 0), QString::fromStdString(propertyArray[i]));
			}

			owner->connect(table, &QTableWidget::cellChanged, [=, &properties, keyName = std::move(keyName), callback = std::forward<CB>(callback)](int row, int column)
			{
				std::string value = table->item(row, column)->text().toStdString();

				PropertyOverrider<true>::template OverrideProperty<P>(owner, arraySize, row, properties, keyName, std::move(value), callback);
			});

			return table;
		}

		template<typename V, typename CB>
		QLineEdit* Setup(EntityInfoDialog* owner, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertySingleValue<P>;

			PropertyValueMap& properties = GetProperties(owner);

			QLineEdit* lineEdit = new QLineEdit;
			if (propertyValue && std::holds_alternative<T>(propertyValue->get()))
				lineEdit->setText(QString::fromStdString(*std::get<T>(propertyValue->get())));

			owner->connect(lineEdit, &QLineEdit::editingFinished, [=, &properties, keyName = std::move(keyName), callback = std::forward<CB>(callback)]()
			{
				std::string value = lineEdit->text().toStdString();

				PropertyOverrider<false>::template OverrideProperty<P>(owner, properties, keyName, std::move(value), callback);
			});

			return lineEdit;
		}
	};


	template<PropertyType P> 
	struct PropertyWidgets;

	template<>
	struct PropertyWidgets<PropertyType::Bool>
	{
		static constexpr PropertyType P = PropertyType::Bool;

		template<typename V, typename CB>
		QTableView* SetupArray(EntityInfoDialog* owner, int arraySize, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertyArrayValue<P>;

			QTableView* tableView = new QTableView;
			QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
			tableView->setModel(model);

			model->setHorizontalHeaderLabels({ owner->tr("Enabled") });

			for (int i = 0; i < arraySize; ++i)
			{
				QStandardItem* item = new QStandardItem(1);
				item->setCheckable(true);

				model->setItem(i, 0, item);
			}

			if (propertyValue)
			{
				auto& propertyArray = std::get<T>(propertyValue->get());
				for (int i = 0; i < arraySize; ++i)
					model->item(i)->setCheckState((propertyArray[i]) ? Qt::Checked : Qt::Unchecked);
			}

			owner->connect(model, &QStandardItemModel::itemChanged, [=, keyName = std::move(keyName), callback = std::forward<CB>(callback)](QStandardItem* item) mutable
			{
				bool value = item->checkState() == Qt::Checked;

				PropertyOverrider<true>::template OverrideProperty<P>(owner, arraySize, item->index().row(), owner->m_entityInfo.properties, keyName, value, callback);
			});

			return tableView;
		}

		template<typename V, typename CB>
		QCheckBox* Setup(EntityInfoDialog* owner, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertySingleValue<P>;

			QCheckBox* checkBox = new QCheckBox;
			if (propertyValue && std::holds_alternative<T>(propertyValue->get()))
				checkBox->setChecked(*std::get<T>(propertyValue->get()));

			owner->connect(checkBox, &QCheckBox::toggled, [=, keyName = std::move(keyName), callback = std::forward<CB>(callback)](bool checked) mutable
			{
				PropertyOverrider<false>::template OverrideProperty<P>(owner, owner->m_entityInfo.properties, keyName, checked, callback);
			});

			return checkBox;
		}
	};

	template<>
	struct PropertyWidgets<PropertyType::Entity>
	{
		static constexpr PropertyType P = PropertyType::Entity;

		template<typename V, typename CB>
		QTableView* SetupArray(EntityInfoDialog* owner, int arraySize, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertyArrayValue<P>;

			owner->m_delegates->comboBoxDelegate.emplace(owner->BuildEntityComboBoxOptions());

			QTableView* tableView = new QTableView;
			QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
			tableView->setItemDelegate(&owner->m_delegates->comboBoxDelegate.value());
			tableView->setModel(model);

			model->setHorizontalHeaderLabels({ owner->tr("Value") });

			if (propertyValue)
			{
				auto& propertyArray = std::get<T>(propertyValue->get());

				for (int i = 0; i < arraySize; ++i)
					owner->m_delegates->comboBoxDelegate->ApplyModelData(model, model->index(i, 0), static_cast<qlonglong>(propertyArray[i]));
			}

			owner->connect(model, &QStandardItemModel::itemChanged, [=, keyName = std::move(keyName), callback = std::forward<CB>(callback)](QStandardItem* item) mutable
			{
				EntityId value = static_cast<EntityId>(owner->m_delegates->comboBoxDelegate->RetrieveModelData(item->index()).toLongLong());
				if (value <= 0)
					value = InvalidEntityId;

				PropertyOverrider<true>::template OverrideProperty<P>(owner, arraySize, item->index().row(), owner->m_entityInfo.properties, keyName, value, callback);
			});

			return tableView;
		}

		template<typename V, typename CB>
		QComboBox* Setup(EntityInfoDialog* owner, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertySingleValue<P>;

			QComboBox* comboBox = new QComboBox;
			for (const auto& option : owner->BuildEntityComboBoxOptions())
				comboBox->addItem(option.first, option.second);

			if (propertyValue && std::holds_alternative<T>(propertyValue->get()))
			{
				EntityId uniqueId = *std::get<T>(propertyValue->get());
				int listSize = comboBox->count();

				int i = 0;
				for (; i < listSize; ++i)
				{
					if (static_cast<EntityId>(comboBox->itemData(i).toLongLong()) == uniqueId)
					{
						comboBox->setCurrentIndex(i);
						break;
					}
				}

				if (i >= listSize)
					comboBox->setCurrentIndex(0);
			}

			owner->connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), [=, keyName = std::move(keyName), callback = std::forward<CB>(callback)](int index)
			{
				EntityId value = static_cast<EntityId>(comboBox->itemData(index).toLongLong());
				if (value <= 0)
					value = InvalidEntityId;

				PropertyOverrider<false>::template OverrideProperty<P>(owner, owner->m_entityInfo.properties, keyName, value, callback);
			});

			return comboBox;
		}
	};

	template<>
	struct PropertyWidgets<PropertyType::Float>
	{
		static constexpr PropertyType P = PropertyType::Float;

		template<typename V, typename CB>
		QTableView* SetupArray(EntityInfoDialog* owner, int arraySize, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertyArrayValue<P>;

			QTableView* tableView = new QTableView;
			QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
			tableView->setItemDelegate(&owner->m_delegates->floatDelegate);
			tableView->setModel(model);

			model->setHorizontalHeaderLabels({ owner->tr("Value") });

			if (propertyValue)
			{
				auto& propertyArray = std::get<T>(propertyValue->get());
				for (int i = 0; i < arraySize; ++i)
					owner->m_delegates->floatDelegate.ApplyModelData(model, model->index(i, 0), propertyArray[i]);
			}

			owner->connect(model, &QStandardItemModel::itemChanged, [=, keyName = std::move(keyName), callback = std::forward<CB>(callback)](QStandardItem* item)
			{
				float value = owner->m_delegates->floatDelegate.RetrieveModelData(item->index());

				PropertyOverrider<true>::template OverrideProperty<P>(owner, arraySize, item->index().row(), owner->m_entityInfo.properties, keyName, value, callback);
			});

			return tableView;
		}

		template<typename V, typename CB>
		QDoubleSpinBox* Setup(EntityInfoDialog* owner, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertySingleValue<P>;

			QDoubleSpinBox* spinbox = new QDoubleSpinBox;
			spinbox->setDecimals(6);
			spinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			if (propertyValue && std::holds_alternative<T>(propertyValue->get()))
				spinbox->setValue(*std::get<T>(propertyValue->get()));

			owner->connect(spinbox, &QDoubleSpinBox::editingFinished, [=, keyName = std::move(keyName), callback = std::forward<CB>(callback)]()
			{
				float value = float(spinbox->value());

				PropertyOverrider<false>::template OverrideProperty<P>(owner, owner->m_entityInfo.properties, keyName, value, callback);
			});

			return spinbox;
		}
	};

	template<>
	struct PropertyWidgets<PropertyType::FloatPosition> final : MultiSpinboxPropertyWidget<PropertyType::FloatPosition, Float2PropertyDelegate, Float2SpinBox, Float2SpinBox::LabelMode::PositionLabel>
	{
		Float2PropertyDelegate* GetDelegate(EntityInfoDialog* owner) override { return &owner->m_delegates->float2Delegate; }
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::FloatPosition3D> final : MultiSpinboxPropertyWidget<PropertyType::FloatPosition3D, Float3PropertyDelegate, Float3SpinBox, Float3SpinBox::LabelMode::PositionLabel>
	{
		Float3PropertyDelegate* GetDelegate(EntityInfoDialog* owner) override { return &owner->m_delegates->float3Delegate; }
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::FloatRect> final : MultiSpinboxPropertyWidget<PropertyType::FloatRect, Float4PropertyDelegate, Float4SpinBox, Float4SpinBox::LabelMode::RectLabel>
	{
		Float4PropertyDelegate* GetDelegate(EntityInfoDialog* owner) override { return &owner->m_delegates->float4Delegate; }
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::FloatSize> final : MultiSpinboxPropertyWidget<PropertyType::FloatSize, Float2PropertyDelegate, Float2SpinBox, Float2SpinBox::LabelMode::SizeLabel>
	{
		Float2PropertyDelegate* GetDelegate(EntityInfoDialog* owner) override { return &owner->m_delegates->float2Delegate; }
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::FloatSize3D> final : MultiSpinboxPropertyWidget<PropertyType::FloatSize3D, Float3PropertyDelegate, Float3SpinBox, Float3SpinBox::LabelMode::SizeLabel>
	{
		Float3PropertyDelegate* GetDelegate(EntityInfoDialog* owner) override { return &owner->m_delegates->float3Delegate; }
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::Integer>
	{
		static constexpr PropertyType P = PropertyType::Integer;

		template<typename V, typename CB>
		QTableView* SetupArray(EntityInfoDialog* owner, int arraySize, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertyArrayValue<P>;

			QTableView* tableView = new QTableView;
			QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
			tableView->setItemDelegate(&owner->m_delegates->intDelegate);
			tableView->setModel(model);

			model->setHorizontalHeaderLabels({ owner->tr("Value") });

			if (propertyValue)
			{
				auto& propertyArray = std::get<T>(propertyValue->get());
				for (int i = 0; i < arraySize; ++i)
					owner->m_delegates->intDelegate.ApplyModelData(model, model->index(i, 0), propertyArray[i]);
			}

			owner->connect(model, &QStandardItemModel::itemChanged, [=, keyName = std::move(keyName), callback = std::forward<CB>(callback)](QStandardItem* item)
			{
				Nz::Int64 value = owner->m_delegates->intDelegate.RetrieveModelData(item->index());

				PropertyOverrider<true>::template OverrideProperty<P>(owner, arraySize, item->index().row(), owner->m_entityInfo.properties, keyName, value, callback);
			});

			return tableView;
		}

		template<typename V, typename CB>
		QSpinBox* Setup(EntityInfoDialog* owner, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertySingleValue<P>;

			// TODO: Handle properly int64
			QSpinBox* spinbox = new QSpinBox;
			spinbox->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());

			if (propertyValue && std::holds_alternative<T>(propertyValue->get()))
				spinbox->setValue(*std::get<T>(propertyValue->get()));

			owner->connect(spinbox, &QSpinBox::editingFinished, [=, keyName = std::move(keyName), callback = std::forward<CB>(callback)]()
			{
				Nz::Int64 value = spinbox->value();

				PropertyOverrider<false>::template OverrideProperty<P>(owner, owner->m_entityInfo.properties, keyName, value, callback);
			});

			return spinbox;
		}
	};

	template<>
	struct PropertyWidgets<PropertyType::IntegerPosition> final : MultiSpinboxPropertyWidget<PropertyType::IntegerPosition, Integer2PropertyDelegate, Integer2SpinBox, Integer2SpinBox::LabelMode::PositionLabel>
	{
		Integer2PropertyDelegate* GetDelegate(EntityInfoDialog* owner) override { return &owner->m_delegates->int2Delegate; }
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::IntegerPosition3D> final : MultiSpinboxPropertyWidget<PropertyType::IntegerPosition3D, Integer3PropertyDelegate, Integer3SpinBox, Integer3SpinBox::LabelMode::PositionLabel>
	{
		Integer3PropertyDelegate* GetDelegate(EntityInfoDialog* owner) override { return &owner->m_delegates->int3Delegate; }
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::IntegerRect> final : MultiSpinboxPropertyWidget<PropertyType::IntegerRect, Integer4PropertyDelegate, Integer4SpinBox, Integer4SpinBox::LabelMode::RectLabel>
	{
		Integer4PropertyDelegate* GetDelegate(EntityInfoDialog* owner) override { return &owner->m_delegates->int4Delegate; }
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::IntegerSize> final : MultiSpinboxPropertyWidget<PropertyType::IntegerSize, Integer2PropertyDelegate, Integer2SpinBox, Integer2SpinBox::LabelMode::SizeLabel>
	{
		Integer2PropertyDelegate* GetDelegate(EntityInfoDialog* owner) override { return &owner->m_delegates->int2Delegate; }
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::IntegerSize3D> final : MultiSpinboxPropertyWidget<PropertyType::IntegerSize3D, Integer3PropertyDelegate, Integer3SpinBox, Integer3SpinBox::LabelMode::SizeLabel>
	{
		Integer3PropertyDelegate* GetDelegate(EntityInfoDialog* owner) override { return &owner->m_delegates->int3Delegate; }
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::Layer>
	{
		static constexpr PropertyType P = PropertyType::Layer;

		template<typename V, typename CB>
		QTableView* SetupArray(EntityInfoDialog* owner, int arraySize, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertyArrayValue<P>;

			owner->m_delegates->comboBoxDelegate.emplace(owner->BuildLayerComboBoxOptions());

			QTableView* tableView = new QTableView;
			QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
			tableView->setItemDelegate(&owner->m_delegates->comboBoxDelegate.value());
			tableView->setModel(model);

			model->setHorizontalHeaderLabels({ owner->tr("Value") });

			if (propertyValue)
			{
				auto& propertyArray = std::get<T>(propertyValue->get());

				for (int i = 0; i < arraySize; ++i)
					owner->m_delegates->comboBoxDelegate->ApplyModelData(model, model->index(i, 0), static_cast<qlonglong>(propertyArray[i]));
			}

			owner->connect(model, &QStandardItemModel::itemChanged, [=, keyName = std::move(keyName), callback = std::forward<CB>(callback)](QStandardItem* item)
			{
				LayerIndex value = static_cast<LayerIndex>(owner->m_delegates->comboBoxDelegate->RetrieveModelData(item->index()).toLongLong());

				PropertyOverrider<true>::template OverrideProperty<P>(owner, arraySize, item->index().row(), owner->m_entityInfo.properties, keyName, value, callback);
			});

			return tableView;
		}

		template<typename V, typename CB>
		QComboBox* Setup(EntityInfoDialog* owner, const V& propertyValue, std::string keyName, CB&& callback)
		{
			using T = PropertySingleValue<P>;

			QComboBox* comboBox = new QComboBox;
			for (const auto& option : owner->BuildLayerComboBoxOptions())
				comboBox->addItem(option.first, option.second);

			if (propertyValue && std::holds_alternative<T>(propertyValue->get()))
			{
				int index = int(*std::get<T>(propertyValue->get()));
				if (index == NoLayer)
					index = 0;
				else
					index++;

				comboBox->setCurrentIndex(index);
			}

			owner->connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), [=, keyName = std::move(keyName), callback = std::forward<CB>(callback)](int index)
			{
				LayerIndex value = static_cast<LayerIndex>(comboBox->itemData(index).toLongLong());

				PropertyOverrider<false>::template OverrideProperty<P>(owner, owner->m_entityInfo.properties, keyName, value, callback);
			});

			return comboBox;
		}
	};

	template<>
	struct PropertyWidgets<PropertyType::String> final : TextPropertyWidget<PropertyType::String>
	{
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};

	template<>
	struct PropertyWidgets<PropertyType::Texture> final : TextPropertyWidget<PropertyType::Texture>
	{
		PropertyValueMap& GetProperties(EntityInfoDialog* owner) override { return owner->m_entityInfo.properties; }
	};


	EntityInfoDialog::EntityInfoDialog(const Logger& logger, const Map& map, EditorEntityStore& clientEntityStore, ScriptingContext& scriptingContext, QWidget* parent) :
	QDialog(parent),
	m_entityTypeIndex(0),
	m_propertyTypeIndex(InvalidIndex),
	m_entityStore(clientEntityStore),
	m_logger(logger),
	m_map(map),
	m_scriptingContext(scriptingContext)
	{
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		m_delegates = std::make_unique<Delegates>();

		m_entityTypeWidget = new QComboBox;
		m_entityStore.ForEachElement([this](const ScriptedEntity& entityData)
		{
			m_entityTypes.emplace_back(entityData.fullName);
		});

		std::sort(m_entityTypes.begin(), m_entityTypes.end());

		for (std::size_t i = 0; i < m_entityTypes.size(); ++i)
			m_entityTypeWidget->addItem(QString::fromStdString(m_entityTypes[i]));

		connect(m_entityTypeWidget, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) 
		{
			OnEntityTypeUpdate();
			m_updateFlags |= EntityInfoUpdate::EntityClass;
		});

		QHBoxLayout* propertyLayout = new QHBoxLayout;

		m_propertiesList = new QTableWidget(0, 3);
		m_propertiesList->setHorizontalHeaderLabels({ tr("Property"), tr("Value"), tr("Required") });
		m_propertiesList->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_propertiesList->setSelectionMode(QAbstractItemView::SingleSelection);
		m_propertiesList->setShowGrid(false);
		m_propertiesList->setEditTriggers(QAbstractItemView::NoEditTriggers);


		connect(m_propertiesList, &QTableWidget::currentCellChanged, [this](int currentRow, int /*currentColumn*/, int previousRow, int /*previousColumn*/)
		{
			if (currentRow < 0 || currentRow == previousRow)
				return;

			RefreshPropertyEditor(currentRow);
		});

		m_propertyTitle = new QLabel;
		m_propertyDescription = new QLabel;

		QVBoxLayout* propertyContentLayout = new QVBoxLayout;

		m_propertyContentWidget = new QWidget;
		m_propertyContentWidget->setMinimumSize(320, 320);

		m_resetDefaultButton = new QPushButton(tr("Restore defaults"));
		connect(m_resetDefaultButton, &QPushButton::released, [this]() { OnResetProperty(); });

		propertyContentLayout->addWidget(m_propertyTitle);
		propertyContentLayout->addWidget(m_propertyDescription);
		propertyContentLayout->addStretch();
		propertyContentLayout->addWidget(m_propertyContentWidget);
		propertyContentLayout->addStretch();
		propertyContentLayout->addWidget(m_resetDefaultButton);

		propertyLayout->addWidget(m_propertiesList);
		propertyLayout->addLayout(propertyContentLayout);

		m_nameWidget = new QLineEdit;
		connect(m_nameWidget, &QLineEdit::textEdited, [this](const QString& text)
		{
			m_entityInfo.entityName = text.toStdString();
			m_updateFlags |= EntityInfoUpdate::EntityName;
		});

		m_positionWidget = new Float2SpinBox(Float2SpinBox::LabelMode::PositionLabel, QBoxLayout::LeftToRight);
		connect(m_positionWidget, &Float2SpinBox::valueChanged, [this](Nz::Vector2f value)
		{
			m_entityInfo.position = value;
			m_updateFlags |= EntityInfoUpdate::PositionRotation;
		});

		m_rotationWidget = new QDoubleSpinBox;
		m_rotationWidget->setDecimals(6);
		m_rotationWidget->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		connect(m_rotationWidget, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double rotation)
		{
			m_entityInfo.rotation = Nz::DegreeAnglef::FromDegrees(float(rotation));
			m_entityInfo.rotation.Normalize();
			m_updateFlags |= EntityInfoUpdate::PositionRotation;
		});

		m_entityIndexLabel = new QLabel;

		QFormLayout* genericPropertyLayout = new QFormLayout;
		genericPropertyLayout->addRow(tr("Entity type"), m_entityTypeWidget);
		genericPropertyLayout->addRow(tr("Entity name"), m_nameWidget);
		genericPropertyLayout->addRow(tr("Entity position"), m_positionWidget);
		genericPropertyLayout->addRow(tr("Entity rotation"), m_rotationWidget);
		genericPropertyLayout->addRow(tr("Entity unique id"), m_entityIndexLabel);

		QDialogButtonBox* button = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(button, &QDialogButtonBox::accepted, this, &EntityInfoDialog::OnAccept);
		connect(button, &QDialogButtonBox::rejected, this, &QDialog::reject);

		m_editorActionWidget = new QWidget;
		m_editorActionLayout = new QHBoxLayout;
		m_editorActionWidget->setLayout(m_editorActionLayout);

		QVBoxLayout* verticalLayout = new QVBoxLayout;
		verticalLayout->addLayout(genericPropertyLayout);
		verticalLayout->addLayout(propertyLayout);
		verticalLayout->addWidget(m_editorActionWidget);
		verticalLayout->addWidget(button);

		setLayout(verticalLayout);

		connect(this, &QDialog::finished, [this](int result)
		{
			if (m_callback)
			{
				if (result == QDialog::Accepted)
					m_callback(this, std::move(m_entityInfo), m_updateFlags);

				m_callback = Callback();
			}
		});

		setWindowTitle("Entity editor");
		hide();
	}

	EntityInfoDialog::~EntityInfoDialog() = default;

	const PropertyValue& EntityInfoDialog::GetProperty(const std::string& propertyName) const
	{
		const bw::PropertyValue* property;

		auto it = m_entityInfo.properties.find(propertyName);
		if (it != m_entityInfo.properties.end())
			property = &it->second;
		else
		{
			auto propertyIt = m_propertyByName.find(propertyName);
			if (propertyIt == m_propertyByName.end())
				throw std::runtime_error("Property " + propertyName + " does not exist");

			const auto& propertyData = m_properties[propertyIt->second];
			if (!propertyData.defaultValue)
				throw std::runtime_error("Property " + propertyName + " has value nor default value");

			property = &propertyData.defaultValue.value();
		}

		return *property;
	}

	std::pair<PropertyType, bool> EntityInfoDialog::GetPropertyType(const std::string & propertyName) const
	{
		auto propertyIt = m_propertyByName.find(propertyName);
		if (propertyIt == m_propertyByName.end())
			throw std::runtime_error("Property " + propertyName + " does not exist");

		const auto& propertyData = m_properties[propertyIt->second];
		return std::make_pair(propertyData.type, propertyData.isArray);
	}

	void EntityInfoDialog::Open(EntityId uniqueId, LayerIndex layerIndex, std::optional<EntityInfo> info, entt::entity targetEntity, Callback callback)
	{
		m_entityLayer = layerIndex;
		m_entityUniqueId = uniqueId;
		m_callback = std::move(callback);
		m_targetEntity = targetEntity;

		if (info)
		{
			// Editing mode
			m_entityInfo = std::move(*info);

			QString oldClass = m_entityTypeWidget->currentText();
			QString newClass = QString::fromStdString(m_entityInfo.entityClass);
			if (oldClass != newClass)
			{
				if (!newClass.isEmpty())
					m_entityTypeWidget->setCurrentIndex(m_entityTypeWidget->findText(newClass));
				else
					m_entityTypeWidget->setCurrentIndex(-1);
			}
			else
				OnEntityTypeUpdate();
		}
		else
		{
			// Creation mode
			m_entityInfo = EntityInfo();

			m_entityTypeWidget->setCurrentIndex(-1);
			m_propertiesList->clearSelection();
		}

		m_nameWidget->setText(QString::fromStdString(m_entityInfo.entityName));
		m_positionWidget->setValue(m_entityInfo.position);
		m_rotationWidget->setValue(m_entityInfo.rotation.ToDegrees());
		m_entityIndexLabel->setText(QString::number(m_entityUniqueId));

		m_updateFlags.Clear();

		QDialog::open();
	}

	void EntityInfoDialog::UpdatePosition(const Nz::Vector2f& position)
	{
		m_positionWidget->setValue(position);
	}

	void EntityInfoDialog::UpdateRotation(const Nz::DegreeAnglef& rotation)
	{
		m_rotationWidget->setValue(rotation.ToDegrees());
	}

	void EntityInfoDialog::UpdateProperty(const std::string& propertyName, PropertyValue propertyValue)
	{
		auto it = m_entityInfo.properties.insert_or_assign(propertyName, std::move(propertyValue)).first;
		m_updateFlags |= EntityInfoUpdate::Properties;

		// Check if we should reload panel
		auto propertyIt = m_propertyByName.find(propertyName);
		assert(propertyIt != m_propertyByName.end());

		std::size_t propertyIndex = propertyIt->second;

		m_propertiesList->item(int(propertyIndex), 1)->setText(ToString(it.value(), m_properties[propertyIndex].type));

		if (m_propertyTypeIndex == propertyIndex)
			RefreshPropertyEditor(m_propertyTypeIndex);
	}

	void EntityInfoDialog::OnEntityTypeUpdate()
	{
		std::string entityType = m_entityTypeWidget->currentText().toStdString();

		m_entityTypeIndex = m_entityStore.GetElementIndex(entityType);

		std::string propertyName;
		if (m_propertyTypeIndex != InvalidIndex)
			propertyName = m_properties[m_propertyTypeIndex].keyName;

		RefreshEntityType();

		if (!propertyName.empty())
		{
			auto propertyIt = m_propertyByName.find(propertyName);
			if (propertyIt != m_propertyByName.end())
				m_propertiesList->selectRow(int(propertyIt->second));
			else
				RefreshPropertyEditor(InvalidIndex);
		}
		else
			RefreshPropertyEditor(InvalidIndex);

		m_entityInfo.entityClass = (m_entityTypeIndex != m_entityStore.InvalidIndex) ? std::move(entityType) : std::string();
	}

	void EntityInfoDialog::OnResetProperty()
	{
		if (m_propertyTypeIndex == InvalidIndex)
			return;

		assert(m_propertyTypeIndex < m_properties.size());
		const auto& propertyInfo = m_properties[m_propertyTypeIndex];
		m_entityInfo.properties.erase(propertyInfo.keyName);

		m_updateFlags |= EntityInfoUpdate::Properties;

		QTableWidgetItem* item = m_propertiesList->item(int(m_propertyTypeIndex), 1);
		item->setFont(QFont());
		item->setText(ToString(GetProperty(propertyInfo), propertyInfo.type));

		RefreshPropertyEditor(m_propertyTypeIndex);
	}

	void EntityInfoDialog::RefreshEntityType()
	{
		m_editorActionByName.clear();
		m_properties.clear();
		m_propertyByName.clear();
		m_propertiesList->clearContents();

		if (m_entityTypeIndex == m_entityStore.InvalidIndex)
		{
			m_entityInfo.properties.clear();
			m_propertiesList->setRowCount(0);
			return;
		}

		auto entityTypeInfo = std::static_pointer_cast<EditorScriptedEntity, ScriptedEntity>(m_entityStore.GetElement(m_entityTypeIndex));

		// Build property list and ensure relevant properties are stored
		PropertyValueMap oldProperties = std::move(m_entityInfo.properties);
		m_entityInfo.properties.clear(); // Put back in a valid state

		std::bitset<MaxPropertyCount> modifiedProperties;

		for (const auto& [propertyName, propertyInfo] : entityTypeInfo->properties)
		{
			auto& propertyData = m_properties.emplace_back();
			propertyData.defaultValue = propertyInfo.defaultValue;
			propertyData.index = propertyInfo.index;
			propertyData.isArray = propertyInfo.isArray;
			propertyData.keyName = propertyName;
			propertyData.visualName = propertyData.keyName; //< FIXME
			propertyData.type = propertyInfo.type;

			if (auto it = oldProperties.find(propertyData.keyName); it != oldProperties.end())
			{
				// Only keep old property value if types are compatibles
				if (!propertyInfo.defaultValue || it->second.index() == propertyInfo.defaultValue->index())
				{
					assert(propertyData.index < modifiedProperties.size());
					modifiedProperties[propertyData.index] = true;
					m_entityInfo.properties.emplace(std::move(it.key()), std::move(it.value()));
				}

				oldProperties.erase(it);
			}
		}

		std::sort(m_properties.begin(), m_properties.end(), [](auto&& first, auto&& second) { return first.index < second.index; });

		for (std::size_t i = 0; i < m_properties.size(); ++i)
			m_propertyByName.emplace(m_properties[i].keyName, i);

		m_propertiesList->setRowCount(int(m_properties.size()));

		QFont boldFont;
		boldFont.setWeight(QFont::Medium);

		int rowIndex = 0;
		for (const auto& propertyInfo : m_properties)
		{
			m_propertiesList->setItem(rowIndex, 0, new QTableWidgetItem(QString::fromStdString(propertyInfo.visualName)));

			QTableWidgetItem* valueItem = new QTableWidgetItem(ToString(GetProperty(propertyInfo), propertyInfo.type));
			if (modifiedProperties.test(propertyInfo.index))
				valueItem->setFont(boldFont);
			
			m_propertiesList->setItem(rowIndex, 1, valueItem);
			m_propertiesList->setItem(rowIndex, 2, new QTableWidgetItem( propertyInfo.defaultValue.has_value() ? "" : "*" ));

			++rowIndex;
		}


		while (QWidget* w = m_editorActionWidget->findChild<QWidget*>())
			delete w;

		std::size_t actionIndex = 0;
		for (auto&& editorAction : entityTypeInfo->editorActions)
		{
			m_editorActionByName.emplace(editorAction.name, actionIndex++);

			QPushButton* button = new QPushButton(QString::fromStdString(editorAction.label));
			connect(button, &QPushButton::released, [this, name = editorAction.name]()
			{
				auto it = m_editorActionByName.find(name);
				assert(it != m_editorActionByName.end());

				auto entityTypeInfo = std::static_pointer_cast<EditorScriptedEntity, ScriptedEntity>(m_entityStore.GetElement(m_entityTypeIndex));

				const auto& action = entityTypeInfo->editorActions[it->second];

				auto result = action.onTrigger(this);
				if (!result.valid())
				{
					sol::error err = result;
					bwLog(m_logger, LogLevel::Error, "Editor action {0}::OnTrigger failed: {1}", name, err.what());
				}
			});

			m_editorActionLayout->addWidget(button);
		}
	}

	void EntityInfoDialog::RefreshPropertyEditor(std::size_t propertyIndex)
	{
		while (QWidget* w = m_propertyContentWidget->findChild<QWidget*>())
			delete w;

		delete m_propertyContentWidget->layout();

		m_propertyTypeIndex = propertyIndex;
		if (m_propertyTypeIndex == InvalidIndex)
			return;

		assert(propertyIndex < m_properties.size());
		const auto& propertyInfo = m_properties[propertyIndex];
		EntityPropertyConstRefOpt propertyValue = GetProperty(propertyInfo);

		m_propertyTitle->setText(QString::fromStdString(propertyInfo.visualName));

		// Only allow to reset if a default value exists
		m_resetDefaultButton->setEnabled(propertyInfo.defaultValue.has_value());

		QVBoxLayout* layout = new QVBoxLayout;

		bool isArray = propertyInfo.isArray;
		int arraySize = 0;

		if (propertyValue)
		{
			std::visit([&](auto&& propertyValue)
			{
				using T = std::decay_t<decltype(propertyValue)>;
				using PropertyTypeExtractor = PropertyTypeExtractor<T>;
				constexpr bool IsArray = PropertyTypeExtractor::IsArray;

				isArray = IsArray;
				if constexpr (IsArray)
					arraySize = int(propertyValue.size());

			}, propertyValue->get());
		}

		auto UpdatePropertyPreview = [this, propertyIndex](const QString& preview)
		{
			m_propertiesList->item(int(propertyIndex), 1)->setText(preview);
		};

		auto OnPropertyOverride = [this, propertyIndex, isArray, UpdatePropertyPreview](bool firstTime, QString preview)
		{
			if (firstTime)
			{
				QFont boldFont;
				boldFont.setWeight(QFont::Medium);

				m_propertiesList->item(int(propertyIndex), 1)->setFont(boldFont);
			}

			if (!preview.isEmpty())
				UpdatePropertyPreview(preview);

			m_updateFlags |= EntityInfoUpdate::Properties;
		};

		if (isArray)
		{
			QSpinBox* spinbox = new QSpinBox;
			spinbox->setRange(0, std::numeric_limits<int>::max());
			spinbox->setValue(arraySize);

			QPushButton* updateButton = new QPushButton(tr("Update"));
			connect(updateButton, &QPushButton::released, [this, &propertyInfo, spinbox, propertyIndex, UpdatePropertyPreview]()
			{
				std::size_t newSize = spinbox->value();

				// Ensure we have a custom value for this
				PropertyValue* property;
				if (auto it = m_entityInfo.properties.find(propertyInfo.keyName); it != m_entityInfo.properties.end())
					property = &it.value();
				else
				{
					if (propertyInfo.defaultValue)
					{
						const PropertyValue& defaultValue = propertyInfo.defaultValue.value();

						auto propertyIt = m_entityInfo.properties.emplace(propertyInfo.keyName, defaultValue);
						property = &propertyIt.first.value();
					}
					else
						property = nullptr;
				}

				if (property)
				{
					std::visit([&](auto&& propertyValue)
					{
						using T = std::decay_t<decltype(propertyValue)>;
						using PropertyTypeExtractor = PropertyTypeExtractor<T>;
						constexpr bool IsArray = PropertyTypeExtractor::IsArray;

						// We have to use if constexpr here because the compiler will instantiate this lambda even for single types
						assert(IsArray);
						if constexpr (IsArray) //< always true
						{
							PropertyArrayValue<PropertyTypeExtractor::Property> newArray(newSize);

							// Copy old values
							std::size_t size = std::min(newArray.size(), propertyValue.size());
							for (std::size_t i = 0; i < size; ++i)
								newArray[i] = propertyValue[i];

							propertyValue = std::move(newArray);
						}

					}, *property);
				}

				RefreshPropertyEditor(propertyIndex);
				UpdatePropertyPreview(ToString(*property, propertyInfo.type));
			});

			QHBoxLayout* arraySizeLayout = new QHBoxLayout;
			arraySizeLayout->addWidget(spinbox);
			arraySizeLayout->addWidget(updateButton);

			layout->addLayout(arraySizeLayout);
			
			switch (propertyInfo.type)
			{
#define BURGWAR_PROPERTYTYPE(V, X, UT) \
				case PropertyType:: X: \
					layout->addWidget(PropertyWidgets<PropertyType:: X>().SetupArray(this, arraySize, propertyValue, propertyInfo.keyName, OnPropertyOverride)); \
					break;

#include <CoreLib/PropertyTypeList.hpp>
			}
		}
		else
		{
			switch (propertyInfo.type)
			{
#define BURGWAR_PROPERTYTYPE(V, X, UT) \
				case PropertyType:: X: \
					layout->addWidget(PropertyWidgets<PropertyType:: X>().Setup(this, propertyValue, propertyInfo.keyName, OnPropertyOverride)); \
					break;

#include <CoreLib/PropertyTypeList.hpp>
			}
		}

		m_propertyContentWidget->setLayout(layout);
	}

	QString EntityInfoDialog::ToString(bool value, PropertyType /*type*/)
	{
		return (value) ? "true" : "false";
	}

	QString EntityInfoDialog::ToString(float value, PropertyType /*type*/)
	{
		return QString::number(value);
	}

	QString EntityInfoDialog::ToString(Nz::UInt16 value, PropertyType type)
	{
		assert(type == PropertyType::Layer);

		if (value == NoLayer)
			return tr("<No layer>");
		else
		{
			if (value >= m_map.GetLayerCount())
				return tr("<Invalid layer> (%1)").arg(value + 1);

			auto& layer = m_map.GetLayer(value);
			return tr("%1 (%2)").arg(QString::fromStdString(layer.name)).arg(value + 1);
		}
	}

	QString EntityInfoDialog::ToString(Nz::Int64 value, PropertyType type)
	{
		assert(type == PropertyType::Entity || type == PropertyType::Integer);
		switch (type)
		{
			case PropertyType::Entity:
			{
				if (value < 0)
					return tr("<No entity>");
				else
				{
					for (std::size_t i = 0; i < m_map.GetLayerCount(); ++i)
					{
						auto& layer = m_map.GetLayer(LayerIndex(i));
						for (std::size_t j = 0; j < layer.entities.size(); ++j)
						{
							auto& entity = layer.entities[j];
							if (entity.uniqueId == value)
							{
								QString layerName = QString::fromStdString(layer.name);
								QString entityName = QString::fromStdString(entity.name);
								if (entityName.isEmpty())
									entityName = tr("<unnamed>");

								return tr("Layer %1 (%2) - Entity %3 (%4) of type %5")
								       .arg(layerName)
								       .arg(i + 1)
								       .arg(entityName)
								       .arg(j + 1)
								       .arg(QString::fromStdString(entity.entityType));
							}
						}
					}

					return tr("<Invalid entity>");
				}
			}

			case PropertyType::Integer:
				return QString::number(value);

			default:
				assert(false);
				return "<error>";
		}
	}

	QString EntityInfoDialog::ToString(const Nz::Vector2f& value, PropertyType /*type*/)
	{
		return QString("(%1; %2)").arg(value.x).arg(value.y);
	}

	QString EntityInfoDialog::ToString(const Nz::Vector2i64& value, PropertyType /*type*/)
	{
		return QString("(%1; %2)").arg(value.x).arg(value.y);
	}

	QString EntityInfoDialog::ToString(const Nz::Vector3f& value, PropertyType /*type*/)
	{
		return QString("(%1; %2; %3)").arg(value.x).arg(value.y).arg(value.z);
	}

	QString EntityInfoDialog::ToString(const Nz::Vector3i64& value, PropertyType /*type*/)
	{
		return QString("(%1; %2; %3)").arg(value.x).arg(value.y).arg(value.z);
	}

	QString EntityInfoDialog::ToString(const Nz::Vector4f& value, PropertyType type)
	{
		if (type == PropertyType::FloatRect)
			return QString("(%1, %2, %3, %4)").arg(value.x).arg(value.y).arg(value.z).arg(value.w);
		else
			return QString("(%1; %2; %3; %4)").arg(value.x).arg(value.y).arg(value.z).arg(value.w);
	}

	QString EntityInfoDialog::ToString(const Nz::Vector4i64& value, PropertyType type)
	{
		if (type == PropertyType::IntegerRect)
			return QString("(%1, %2, %3, %4)").arg(value.x).arg(value.y).arg(value.z).arg(value.w);
		else
			return QString("(%1; %2; %3; %4)").arg(value.x).arg(value.y).arg(value.z).arg(value.w);
	}

	QString EntityInfoDialog::ToString(const std::string& value, PropertyType /*type*/)
	{
		return QString::fromStdString(value);
	}

	QString EntityInfoDialog::ToString(EntityPropertyConstRefOpt property, PropertyType type)
	{
		if (!property)
			return QString("<No value>");

		return std::visit([=](const auto& value) -> QString
		{
			using T = std::decay_t<decltype(value)>;
			using PropertyTypeExtractor = PropertyTypeExtractor<T>;
			constexpr bool IsArray = PropertyTypeExtractor::IsArray;

			// We have to use if constexpr here because the compiler will instantiate this lambda even for single types
			if constexpr (IsArray)
			{
				return "Array of " + QString::number(value.size());
			}
			else
			{
				return ToString(value.value, type);
			}
		}, property->get());
	}

	std::vector<std::pair<QString, QVariant>> EntityInfoDialog::BuildEntityComboBoxOptions()
	{
		std::vector<std::pair<QString, QVariant>> options;
		options.emplace_back(tr("<No entity>"), static_cast<qlonglong>(InvalidEntityId));

		for (std::size_t i = 0; i < m_map.GetLayerCount(); ++i)
		{
			auto& layer = m_map.GetLayer(LayerIndex(i));

			QString layerName = QString::fromStdString(layer.name);

			for (std::size_t j = 0; j < layer.entities.size(); ++j)
			{
				auto& entity = layer.entities[j];

				QString entityName = QString::fromStdString(entity.name);
				if (entityName.isEmpty())
					entityName = tr("<unnamed>");

				options.emplace_back(tr("Layer %1 (%2) - Entity %3 (%4) of type %5")
				                    .arg(layerName)
				                    .arg(i + 1)
				                    .arg(entityName)
				                    .arg(j + 1)
				                    .arg(QString::fromStdString(entity.entityType)), static_cast<qlonglong>(entity.uniqueId));
			}
		}

		return options;
	}

	std::vector<std::pair<QString, QVariant>> EntityInfoDialog::BuildLayerComboBoxOptions()
	{
		std::vector<std::pair<QString, QVariant>> options;
		options.emplace_back(tr("<No layer>"), NoLayer);

		for (std::size_t i = 0; i < m_map.GetLayerCount(); ++i)
		{
			auto& layer = m_map.GetLayer(LayerIndex(i));

			options.emplace_back(tr("%1 (%2)").arg(QString::fromStdString(layer.name)).arg(i + 1), LayerIndex(i));
		}

		return options;
	}

	auto EntityInfoDialog::GetProperty(const PropertyData& property) const -> EntityPropertyConstRefOpt
	{
		if (auto it = m_entityInfo.properties.find(property.keyName); it != m_entityInfo.properties.end())
			return it->second;
		else
			return property.defaultValue;
	}

	void EntityInfoDialog::OnAccept()
	{
		if (m_entityTypeWidget->currentIndex() < 0)
		{
			QMessageBox::critical(this, tr("Invalid entity type"), tr("You must select a valid entity type"), QMessageBox::Ok);
			return;
		}

		for (const auto& propertyInfo : m_properties)
		{
			if (!propertyInfo.defaultValue)
			{
				if (m_entityInfo.properties.find(propertyInfo.keyName) == m_entityInfo.properties.end())
				{
					QMessageBox::critical(this, tr("Missing required property"), tr("Property %1 has no value (mandatory field)").arg(QString::fromStdString(propertyInfo.visualName)), QMessageBox::Ok);
					return;
				}
			}
		}

		accept();
	}
}
